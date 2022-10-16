#include "seaport_plugin.h"

static uint8_t get_basic_order_type(uint8_t basic_order_type_sol)
{
  uint8_t type = 0;
  switch (basic_order_type_sol)
  {
  case 0:
  case 1:
  case 2:
  case 3:
  case 4:
  case 5:
  case 6:
  case 7:
    type = ETH_NFT;
    break;
  case 8:
  case 9:
  case 10:
  case 11:
  case 12:
  case 13:
  case 14:
  case 15:
    type = ERC20_NFT;
    break;
  case 16:
  case 17:
  case 18:
  case 19:
  case 20:
  case 21:
  case 22:
  case 23:
    type = NFT_ERC20;
    break;
  default:
    PRINTF("undefined basic_order_type_sol\n");
    // msg->result = ETH_PLUGIN_RESULT_ERROR;
    break;
  }
  return type;
}

/**
 * parse both create, processInputOrder and processOutputOrder.
 */
static void handle_fulfill_basic_order(ethPluginProvideParameter_t *msg,
                                       context_t *context)
{
  // Switch to current struct parsing function.
  // if (context->on_struct) {
  //   if (context->on_struct == S_BATCHED_INPUT_ORDERS)
  //     parse_batched_input_orders(msg, context);
  //   else if (context->on_struct == S_BATCHED_OUTPUT_ORDERS)
  //     parse_batched_output_orders(msg, context);
  //   else if (context->on_struct == S_ORDER)
  //     parse_order(msg, context);
  //   else {
  //     PRINTF("handle_create on_struct ERROR\n");
  //     msg->result = ETH_PLUGIN_RESULT_ERROR;
  //   }
  //   return;
  // }
  switch ((fulfill_basic_order_parameter)context->next_param)
  {
  case FBO__OFFSET_BASIC_ORDER_PARAM:
    PRINTF("FBO__OFFSET_BASIC_ORDER_PARAM\n");
    context->next_param = FBO__CONSIDERATION_TOKEN;
    break;
  case FBO__CONSIDERATION_TOKEN:
    PRINTF("FBO__CONSIDERATION_TOKEN\n");
    copy_address(context->token2_address, msg->parameter, ADDRESS_LENGTH);
    context->next_param = FBO__CONSIDERATION_IDENTIFIER;
    break;
  case FBO__CONSIDERATION_IDENTIFIER:
    PRINTF("FBO__CONSIDERATION_IDENTIFIER\n");
    context->next_param = FBO__CONSIDERATION_AMOUNT;
    break;
  case FBO__CONSIDERATION_AMOUNT:
    PRINTF("FBO__CONSIDERATION_AMOUNT\n");
    copy_parameter(context->token1_amount, msg->parameter, PARAMETER_LENGTH);
    context->next_param = FBO__OFFERER;
    break;
  case FBO__OFFERER:
    PRINTF("FBO__OFFERER\n");
    context->skip = 1;
    context->next_param = FBO__OFFER_TOKEN;
    break;
  case FBO__OFFER_TOKEN:
    PRINTF("FBO__OFFER_TOKEN\n");
    copy_address(context->token2_address, msg->parameter, PARAMETER_LENGTH);
    context->next_param = FBO__OFFER_IDENTIFIER;
    break;
  case FBO__OFFER_IDENTIFIER:
    PRINTF("FBO__OFFER_IDENTIFIER\n");
    // copy_parameter(context->nft_id, msg->parameter, PARAMETER_LENGTH);
    context->next_param = FBO__OFFER_AMOUNT;
    break;
  case FBO__OFFER_AMOUNT:
    PRINTF("FBO__OFFER_AMOUNT\n");
    copy_parameter(context->token2_amount, msg->parameter, PARAMETER_LENGTH);
    context->next_param = FBO__BASIC_ORDER_TYPE;
    break;
  case FBO__BASIC_ORDER_TYPE:
    PRINTF("FBO__BASIC_ORDER_TYPE\n");
    context->basic_order_type =
        get_basic_order_type(U2BE(msg->parameter, PARAMETER_LENGTH - 2));
    context->skip = 9;
    context->next_param = FBO__LEN_ADDITIONAL_RECIPIENTS;
    break;
  case FBO__LEN_ADDITIONAL_RECIPIENTS:
    PRINTF("FBO__LEN_ADDITIONAL_RECIPIENTS\n");
    context->current_length = U2BE(msg->parameter, PARAMETER_LENGTH - 2);
    context->next_param = FBO__ADDITIONAL_AMOUNT;
    break;
  case FBO__ADDITIONAL_AMOUNT:
    PRINTF("FBO__ADDITIONAL_AMOUNT\n");
    if (context->basic_order_type != NFT_ERC20)
    {
      uint8_t buf_amount[INT256_LENGTH] = {0};
      copy_parameter(buf_amount, msg->parameter, PARAMETER_LENGTH);
      if (add_uint256(context->token1_amount, buf_amount))
      {
        PRINTF("uint256 overflow error.\n");
        msg->result = ETH_PLUGIN_RESULT_ERROR;
      }
    }
    if (0 == --context->current_length)
      context->next_param = FBO__LEN_SIGNATURE;
    context->skip = 1;
    break;
  case FBO__LEN_SIGNATURE:
    PRINTF("FBO__LEN_SIGNATURE\n");
    break;
  default:
    PRINTF("Param not supported: %d\n", context->next_param);
    msg->result = ETH_PLUGIN_RESULT_ERROR;
    break;
  }
}

//static void parse_fulfill_order_item(ethPluginProvideParameter_t *msg, context_t *context)
//{
//  switch ((fulfill_order_offer_item_type)context->enum_param)
//  {
//  }
//}

static void print_item(context_t *context)
{
  PRINTF("PRINT ITEMS\n");
  if (context->offer_item_type == OFFER_ITEM_TYPE_NONE)
    PRINTF("ITEM TYPE FOUND: NONE\n", context->offer_item_type);

  if (context->offer_item_type == OFFER_ITEM_TYPE_NATIVE)
    PRINTF("ITEM TYPE FOUND: NATIVE\n", context->offer_item_type);

  if (context->offer_item_type == OFFER_ITEM_TYPE_ERC20S)
    PRINTF("ITEM TYPE FOUND: ERC20\n", context->offer_item_type);

  if (context->offer_item_type == OFFER_ITEM_TYPE_NFT)
    PRINTF("ITEM TYPE FOUND: NFT\n", context->offer_item_type);

  if (context->offer_item_type == OFFER_ITEM_TYPE_MULTIPLE_NFTS)
    PRINTF("ITEM TYPE FOUND: MULTIPLE NFTS\n", context->offer_item_type);

  if (context->offer_item_type == OFFER_ITEM_TYPE_MULTIPLE_ERC20S)
    PRINTF("ITEM TYPE FOUND: MULTIPLE_ERC20S\n", context->offer_item_type);

  if (context->offer_item_type == OFFER_ITEM_TYPE_MIXED_TYPES)
    PRINTF("ITEM TYPE FOUND: MIXED TYPES\n", context->offer_item_type);
}

static void parse_fulfill_order_offer(ethPluginProvideParameter_t *msg, context_t *context)
{
  PRINTF("PARSE OFFER\n");
  switch ((fulfill_order_offer)context->enum_param)
  {
  case FO_OFFER_LEN_INIT:
    PRINTF("FO_OFFER_LEN_INIT\n");
    context->number_of_nfts = context->current_length;
    context->enum_param = FO_OFFER_ITEM_TYPE;
    break;
  case FO_OFFER_ITEM_TYPE:
    PRINTF("FO_OFFER_ITEM_TYPE\n");
    PRINTF("OFFER ITEM TYPE CURRENT_LENGTH:%d\n", context->current_length);
    if (context->offer_item_type == OFFER_ITEM_TYPE_NONE)
    {
      if (U2BE(msg->parameter, PARAMETER_LENGTH - 2) + 1 > 1)
      {
        context->offer_item_type = OFFER_ITEM_TYPE_NFT;
      }
      else
        context->offer_item_type = U2BE(msg->parameter, PARAMETER_LENGTH - 2) + 1;
    }
    else if (!(U2BE(msg->parameter, PARAMETER_LENGTH - 2) + 1 > 1) && context->offer_item_type == OFFER_ITEM_TYPE_NFT)
    {
      context->offer_item_type = OFFER_ITEM_TYPE_MIXED_TYPES;
      ////////// IF MIXED TYPES NUMBER OF TOKENS NOT TRUSTABLE
    }
    print_item(context); // utilitary
    context->enum_param = FO_OFFER_TOKEN;
    break;
  case FO_OFFER_TOKEN:
    PRINTF("FO_OFFER_TOKEN\n");
    if (context->number_of_nfts == context->current_length)
    {
      PRINTF("COPY ADDRESS\n");
      copy_address(context->token1_address, msg->parameter, ADDRESS_LENGTH);
    }
    if (memcmp(context->token1_address, msg->parameter + 12, ADDRESS_LENGTH))
    {
      if (context->offer_item_type == OFFER_ITEM_TYPE_NFT)
      {
        PRINTF("OFFER_ITEM_TYPE_MULTIPLE_NFTS\n");
        context->offer_item_type = OFFER_ITEM_TYPE_MULTIPLE_NFTS;
      }
      else
      {
        PRINTF("OFFER_ITEM_TYPE_MULTIPLE_ERC20S\n");
        context->offer_item_type = OFFER_ITEM_TYPE_MULTIPLE_ERC20S;
      }
    }
    context->enum_param = FO_OFFER_IDENTIFIER;
    break;
  case FO_OFFER_IDENTIFIER:
    PRINTF("FO_OFFER_IDENTIFIER\n");
    context->enum_param = FO_OFFER_START_AMOUNT;
    break;
  case FO_OFFER_START_AMOUNT:
    PRINTF("FO_OFFER_START_AMOUNT\n");
    context->enum_param = FO_OFFER_END_AMOUNT;
    break;
  case FO_OFFER_END_AMOUNT:
    PRINTF("FO_OFFER_END_AMOUNT\n");
    context->current_length--;
    context->enum_param = FO_OFFER_ITEM_TYPE;
    break;
  default:
    PRINTF("Param not supported: %d\n", context->next_param);
    msg->result = ETH_PLUGIN_RESULT_ERROR;
    break;
  }
}

static void parse_fulfill_order_consideration(ethPluginProvideParameter_t *msg, context_t *context)
{
  PRINTF("PARSE CONSIDERATION\n");
  switch ((fulfill_order_consideration)context->enum_param)
  {
  case FO_CONSIDERATION_LEN_INIT:
    PRINTF("FO_CONSIDERATION_LEN_INIT\n");
    context->enum_param = FO_CONSIDERATION_ITEM_TYPE;
    break;
  case FO_CONSIDERATION_ITEM_TYPE:
    PRINTF("FO_CONSIDERATION_ITEM_TYPE\n");
    // PRINTF("OFFER ITEM TYPE:%d\n", context->offer_item_type);
    context->enum_param = FO_CONSIDERATION_TOKEN;
    break;
  case FO_CONSIDERATION_TOKEN:
    PRINTF("FO_CONSIDERATION_TOKEN\n");
    context->enum_param = FO_CONSIDERATION_IDENTIFIER;
    break;
  case FO_CONSIDERATION_IDENTIFIER:
    PRINTF("FO_CONSIDERATION_IDENTIFIER\n");
    context->enum_param = FO_CONSIDERATION_START_AMOUNT;
    break;
  case FO_CONSIDERATION_START_AMOUNT:
    PRINTF("FO_CONSIDERATION_START_AMOUNT\n");
    //if () CREATE GET AMOUNT BASED OFF ITEM TYPE
    //{
    //  uint8_t buf_amount[INT256_LENGTH] = {0};
    //  copy_parameter(buf_amount, msg->parameter, PARAMETER_LENGTH);
    //  if (context->offer_item_type != ITEM_TYPE_NFT)
    //    if (add_uint256(context->token2_amount, buf_amount))
    //    {
    //      PRINTF("uint256 overflow error.\n");
    //      msg->result = ETH_PLUGIN_RESULT_ERROR;
    //    }
    //}
    context->enum_param = FO_CONSIDERATION_END_AMOUNT;
    break;
  case FO_CONSIDERATION_END_AMOUNT:
    PRINTF("FO_CONSIDERATION_END_AMOUNT\n");
    context->enum_param = FO_CONSIDERATION_RECIPIENT;
    break;
  case FO_CONSIDERATION_RECIPIENT:
    PRINTF("FO_CONSIDERATION_RECIPIENT\n");
    context->current_length--;
    context->enum_param = FO_CONSIDERATION_ITEM_TYPE;
    break;
  default:
    PRINTF("Param not supported: %d\n", context->next_param);
    msg->result = ETH_PLUGIN_RESULT_ERROR;
    break;
  }
}

static void handle_fullfill_order(ethPluginProvideParameter_t *msg,
                                  context_t *context)
{
  switch ((fullfill_order_parameter)context->next_param)
  {
  case FO_ORDER_OFFSET:
    PRINTF("FO_ORDER_OFFSET\n");
    context->next_param = FO_FULFILLER_CONDUIT_KEY;
    break;
  case FO_FULFILLER_CONDUIT_KEY:
    PRINTF("FO_FULFILLER_CONDUIT_KEY\n");
    context->next_param = FO_ORDER_PARAM_OFFSET;
    break;
  case FO_ORDER_PARAM_OFFSET:
    PRINTF("FO_ORDER_PARAM_OFFSET\n");
    context->next_param = FO_ORDER_SIGNATURE_OFFSET;
    break;
  case FO_ORDER_SIGNATURE_OFFSET:
    PRINTF("FO_ORDER_SIGNATURE_OFFSET\n");
    context->next_param = FO_ORDER_PARAM_OFFERER;
    break;
  case FO_ORDER_PARAM_OFFERER:
    PRINTF("FO_ORDER_PARAM_OFFERER\n");
    copy_address(context->offerer_address, msg->parameter, ADDRESS_LENGTH);
    context->next_param = FO_ORDER_PARAM_ZONE;
    break;
  case FO_ORDER_PARAM_ZONE:
    PRINTF("FO_ORDER_PARAM_ZONE\n");
    context->next_param = FO_ORDER_PARAM_OFFER_OFFSET;
    break;
  case FO_ORDER_PARAM_OFFER_OFFSET:
    PRINTF("FO_ORDER_PARAM_OFFSET\n");
    context->next_param = FO_ORDER_PARAM_CONSIDERATION_OFFSET;
    break;
  case FO_ORDER_PARAM_CONSIDERATION_OFFSET:
    PRINTF("FO_ORDER_PARAM_CONSIDERATION_OFFSET\n");
    context->next_param = FO_ORDER_PARAM_ORDER_TYPE;
    break;
  case FO_ORDER_PARAM_ORDER_TYPE:
    PRINTF("FO_ORDER_PARAM_ORDER_TYPE\n");
    context->next_param = FO_ORDER_PARAM_START_TIME;
    break;
  case FO_ORDER_PARAM_START_TIME:
    PRINTF("FO_ORDER_PARAM_START_TIME\n");
    context->next_param = FO_ORDER_PARAM_END_TIME;
    break;
  case FO_ORDER_PARAM_END_TIME:
    PRINTF("FO_ORDER_PARAM_END_TIME\n");
    context->next_param = FO_ORDER_PARAM_ZONE_HASH;
    break;
  case FO_ORDER_PARAM_ZONE_HASH:
    PRINTF("FO_ORDER_PARAM_ZONE_HASH\n");
    context->next_param = FO_ORDER_PARAM_SALT;
    break;
  case FO_ORDER_PARAM_SALT:
    PRINTF("FO_ORDER_PARAM_SALT\n");
    context->next_param = FO_ORDER_PARAM_CONDUIT_KEY;
    break;
  case FO_ORDER_PARAM_CONDUIT_KEY:
    PRINTF("FO_ORDER_PARAM_CONDUIT_KEY\n");
    context->next_param = FO_ORDER_PARAM_TOTAL_ORIGINAL_CONSIDERATION_ITEMS;
    break;
  case FO_ORDER_PARAM_TOTAL_ORIGINAL_CONSIDERATION_ITEMS:
    PRINTF("FO_ORDER_PARAM_TOTAL_ORIGINAL_CONSIDERATION_ITEMS\n");
    context->next_param = FO_ORDER_PARAM_OFFER_LEN;
    context->enum_param = 0;
    break;
  case FO_ORDER_PARAM_OFFER_LEN:
    PRINTF("FO_ORDER_PARAM_OFFER_LEN\n");
    if (context->enum_param == FO_OFFER_LEN_INIT)
    {
      context->current_length = U2BE(msg->parameter, PARAMETER_LENGTH - 2);
    }
    PRINTF("OFFER CURRENT_LEN:%d\n", context->current_length);
    if (context->current_length > 0)
    {
      parse_fulfill_order_offer(msg, context);
    }
    if (context->current_length == 0)
    {
      context->next_param = FO_ORDER_PARAM_CONSIDERATION_LEN;
      context->enum_param = 0;
    }
    break;
  case FO_ORDER_PARAM_CONSIDERATION_LEN:
    PRINTF("FO_ORDER_PARAM_CONSIDERATION_LEN\n");
    if (context->enum_param == FO_CONSIDERATION_LEN_INIT)
    {
      context->current_length = U2BE(msg->parameter, PARAMETER_LENGTH - 2);
    }
    PRINTF("CONSIDERATION CURRENT_LEN:%d\n", context->current_length);
    if (context->current_length > 0)
    {
      parse_fulfill_order_consideration(msg, context);
    }
    if (context->current_length == 0)
    {
      context->next_param = FO_ORDER_SIGNATURE;
      context->enum_param = 0;
    }
    break;
  case FO_ORDER_SIGNATURE:
    PRINTF("FO_ORDER_SIGNATURE\n");
    break;
  default:
    PRINTF("Param not supported: %d\n", context->next_param);
    msg->result = ETH_PLUGIN_RESULT_ERROR;
    break;
  }
}

void handle_provide_parameter(void *parameters)
{
  ethPluginProvideParameter_t *msg = (ethPluginProvideParameter_t *)parameters;
  context_t *context = (context_t *)msg->pluginContext;
  // We use `%.*H`: it's a utility function to print bytes. You first give
  // the number of bytes you wish to print (in this case, `PARAMETER_LENGTH`)
  // and then the address (here `msg->parameter`).
  PRINTF("___\nplugin provide parameter: offset %d\nBytes: \033[0;31m %.*H \n"
         "\033[0m \n",
         msg->parameterOffset, PARAMETER_LENGTH, msg->parameter);

  msg->result = ETH_PLUGIN_RESULT_OK;

  if (context->skip > 0)
  {
    PRINTF("SKIPPED\n");
    context->skip--;
    return;
  }

  switch (context->selectorIndex)
  {
  case FULFILL_BASIC_ORDER:
    handle_fulfill_basic_order(msg, context);
    break;
  case FULFILL_ORDER:
    handle_fullfill_order(msg, context);
    break;
  default:
    PRINTF("Selector Index not supported: %d\n", context->selectorIndex);
    msg->result = ETH_PLUGIN_RESULT_ERROR;
    break;
  }
}
