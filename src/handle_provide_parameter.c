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
//  switch ((offer_item_type)context->items_index)
//  {
//  }
//}

static void print_item(context_t *context)
{
  if (context->offer_item_type == OFFER_ITEM_TYPE_NONE)
    PRINTF("OFFER ITEM TYPE FOUND: NONE\n", context->offer_item_type);

  if (context->offer_item_type == OFFER_ITEM_TYPE_NATIVE)
    PRINTF("OFFER ITEM TYPE FOUND: NATIVE\n", context->offer_item_type);

  if (context->offer_item_type == OFFER_ITEM_TYPE_ERC20)
    PRINTF("OFFER ITEM TYPE FOUND: ERC20\n", context->offer_item_type);

  if (context->offer_item_type == OFFER_ITEM_TYPE_NFT)
    PRINTF("OFFER ITEM TYPE FOUND: NFT\n", context->offer_item_type);

  if (context->offer_item_type == OFFER_ITEM_TYPE_MULTIPLE_NFTS)
    PRINTF("OFFER ITEM TYPE FOUND: MULTIPLE NFTS\n", context->offer_item_type);

  if (context->offer_item_type == OFFER_ITEM_TYPE_MULTIPLE_ERC20S)
    PRINTF("OFFER ITEM TYPE FOUND: MULTIPLE_ERC20S\n", context->offer_item_type);

  if (context->offer_item_type == OFFER_ITEM_TYPE_MIXED_TYPES)
    PRINTF("OFFER ITEM TYPE FOUND: MIXED TYPES\n", context->offer_item_type);

  if (context->consideration_item_type == CONSIDERATION_ITEM_TYPE_NONE)
    PRINTF("CONSIDERATION ITEM TYPE FOUND: NONE\n", context->consideration_item_type);

  if (context->consideration_item_type == CONSIDERATION_ITEM_TYPE_NATIVE)
    PRINTF("CONSIDERATION ITEM TYPE FOUND: NATIVE\n", context->consideration_item_type);

  if (context->consideration_item_type == CONSIDERATION_ITEM_TYPE_ERC20)
    PRINTF("CONSIDERATION ITEM TYPE FOUND: ERC20\n", context->consideration_item_type);

  if (context->consideration_item_type == CONSIDERATION_ITEM_TYPE_NFT)
    PRINTF("CONSIDERATION ITEM TYPE FOUND: NFT\n", context->consideration_item_type);

  if (context->consideration_item_type == CONSIDERATION_ITEM_TYPE_MULTIPLE_NFTS)
    PRINTF("CONSIDERATION ITEM TYPE FOUND: MULTIPLE NFTS\n", context->consideration_item_type);

  if (context->consideration_item_type == CONSIDERATION_ITEM_TYPE_MULTIPLE_ERC20S)
    PRINTF("CONSIDERATION ITEM TYPE FOUND: MULTIPLE_ERC20S\n", context->consideration_item_type);

  if (context->consideration_item_type == CONSIDERATION_ITEM_TYPE_MIXED_TYPES)
    PRINTF("CONSIDERATION ITEM TYPE FOUND: MIXED TYPES\n", context->consideration_item_type);
  context->sale_side == 0 ? PRINTF("BUY_NOW\n") : PRINTF("ACCEPT_OFFER\n");
}

static void parse_offer(ethPluginProvideParameter_t *msg, context_t *context)
{
  PRINTF("PARSE OFFER\n");
  switch ((offers)context->items_index)
  {
  case OFFER_ITEM_TYPE:
    PRINTF("OFFER_ITEM_TYPE\n");
    PRINTF("OFFER ITEM TYPE CURRENT_LENGTH:%d\n", context->current_length);
    if (context->offer_item_type == OFFER_ITEM_TYPE_NONE)
    {
      if (U2BE(msg->parameter, PARAMETER_LENGTH - 2) > 1)
      {
        PRINTF("OFFER_ITEM_TYPE_NFT\n");
        context->offer_item_type = OFFER_ITEM_TYPE_NFT;
      }
      else
        context->offer_item_type = U2BE(msg->parameter, PARAMETER_LENGTH - 2) + 1;
    }
    if (context->offer_item_type > 1)
    {
      PRINTF("BUY_NOW\n");
      context->sale_side = BUY_NOW;
    }
    else
    {
      PRINTF("ACCEPT_OFFER\n");
      context->sale_side = ACCEPT_OFFER;
    }
    print_item(context); // utilitary
    context->items_index = OFFER_TOKEN;
    break;
  case OFFER_TOKEN:
    PRINTF("OFFER_TOKEN\n");
    if (!memcmp(context->token1_address, NULL_ADDRESS, ADDRESS_LENGTH)) ///// TB FIXED
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
      else if (context->offer_item_type == OFFER_ITEM_TYPE_NATIVE || context->offer_item_type == OFFER_ITEM_TYPE_ERC20)
      {
        PRINTF("OFFER_ITEM_TYPE_MULTIPLE_ERC20S\n");
        context->offer_item_type = OFFER_ITEM_TYPE_MULTIPLE_ERC20S;
      }
    }
    context->items_index = OFFER_IDENTIFIER;
    break;
  case OFFER_IDENTIFIER:
    PRINTF("OFFER_IDENTIFIER\n");
    context->items_index = OFFER_START_AMOUNT;
    break;
  case OFFER_START_AMOUNT:
    PRINTF("OFFER_START_AMOUNT\n");
    uint8_t buf_amount[INT256_LENGTH] = {0};
    copy_parameter(buf_amount, msg->parameter, PARAMETER_LENGTH);
    PRINTF("BUF AMOUNT:\t%.*H\n", INT256_LENGTH, buf_amount);
    if (context->offer_item_type == OFFER_ITEM_TYPE_NATIVE || context->offer_item_type == OFFER_ITEM_TYPE_ERC20) /// TB Adapted to sale side
    {
      PRINTF("SUM\n");
      if (add_uint256(context->token1_amount, buf_amount))
      {
        PRINTF("uint256 overflow error.\n");
        msg->result = ETH_PLUGIN_RESULT_ERROR;
      }
    }
    else if (context->offer_item_type == OFFER_ITEM_TYPE_NFT || context->offer_item_type == OFFER_ITEM_TYPE_MULTIPLE_NFTS)
      context->number_of_nfts += U2BE(msg->parameter, PARAMETER_LENGTH - 2);
    context->items_index = OFFER_END_AMOUNT;
    break;
  case OFFER_END_AMOUNT:
    PRINTF("OFFER_END_AMOUNT\n");
    context->current_length--;
    context->items_index = OFFER_ITEM_TYPE;
    break;
  default:
    PRINTF("Param not supported: %d\n", context->next_param);
    msg->result = ETH_PLUGIN_RESULT_ERROR;
    break;
  }
}

static void parse_considerations(ethPluginProvideParameter_t *msg, context_t *context)
{
  PRINTF("PARSE CONSIDERATION\n");
  switch ((considerations)context->items_index)
  {
  case CONSIDERATION_ITEM_TYPE:
    PRINTF("CONSIDERATION_ITEM_TYPE\n");
    if (context->consideration_item_type == CONSIDERATION_ITEM_TYPE_NONE)
    {
      PRINTF("SET CONSIDERATION ITEM\n");
      if (U2BE(msg->parameter, PARAMETER_LENGTH - 2) + 1 > 1)
      {
        context->consideration_item_type = CONSIDERATION_ITEM_TYPE_NFT;
      }
      else
        context->consideration_item_type = U2BE(msg->parameter, PARAMETER_LENGTH - 2) + 1;
    }
    print_item(context); // utilitary
    context->items_index = CONSIDERATION_TOKEN;
    break;
  case CONSIDERATION_TOKEN:
    PRINTF("CONSIDERATION_TOKEN\n");
    if (!memcmp(context->token2_address, NULL_ADDRESS, ADDRESS_LENGTH)) ///// TB FIXED (if native, pointless repetitive cmp)
    {
      PRINTF("COPY ADDRESS\n");
      copy_address(context->token2_address, msg->parameter, ADDRESS_LENGTH);
    }
    if (memcmp(context->token2_address, msg->parameter + 12, ADDRESS_LENGTH))
    {
      if (context->consideration_item_type == CONSIDERATION_ITEM_TYPE_NFT)
      {
        PRINTF("CONSIDERATION_ITEM_TYPE_MULTIPLE_NFTS\n");
        context->consideration_item_type = CONSIDERATION_ITEM_TYPE_MULTIPLE_NFTS;
      }
      else if (context->consideration_item_type == CONSIDERATION_ITEM_TYPE_NATIVE || context->consideration_item_type == CONSIDERATION_ITEM_TYPE_ERC20)
      {
        PRINTF("CONSIDERATION_ITEM_TYPE_MULTIPLE_ERC20S\n");
        context->consideration_item_type = CONSIDERATION_ITEM_TYPE_MULTIPLE_ERC20S;
      }
    }
    context->items_index = CONSIDERATION_IDENTIFIER;
    break;
  case CONSIDERATION_IDENTIFIER:
    PRINTF("CONSIDERATION_IDENTIFIER\n");
    context->items_index = CONSIDERATION_START_AMOUNT;
    break;
  case CONSIDERATION_START_AMOUNT:
    PRINTF("CONSIDERATION_START_AMOUNT\n");
    uint8_t buf_amount[INT256_LENGTH] = {0};
    copy_parameter(buf_amount, msg->parameter, PARAMETER_LENGTH);
    PRINTF("BUF AMOUNT:\t%.*H\n", INT256_LENGTH, buf_amount);
    if (context->consideration_item_type == CONSIDERATION_ITEM_TYPE_NATIVE || context->consideration_item_type == CONSIDERATION_ITEM_TYPE_ERC20) /// 2B updated with sale side.
    {
      PRINTF("SUM\n");
      if (add_uint256(context->token1_amount, buf_amount))
      {
        PRINTF("uint256 overflow error.\n");
        msg->result = ETH_PLUGIN_RESULT_ERROR;
      }
    }
    else if (context->consideration_item_type == CONSIDERATION_ITEM_TYPE_NFT || context->consideration_item_type == CONSIDERATION_ITEM_TYPE_MULTIPLE_NFTS)
      context->number_of_nfts += U2BE(msg->parameter, PARAMETER_LENGTH - 2);
    context->items_index = CONSIDERATION_END_AMOUNT;
    break;
  case CONSIDERATION_END_AMOUNT:
    PRINTF("CONSIDERATION_END_AMOUNT\n");
    context->items_index = CONSIDERATION_RECIPIENT;
    break;
  case CONSIDERATION_RECIPIENT:
    PRINTF("CONSIDERATION_RECIPIENT\n");
    context->current_length--;
    context->items_index = CONSIDERATION_ITEM_TYPE;
    break;
  default:
    PRINTF("Param not supported: %d\n", context->next_param);
    msg->result = ETH_PLUGIN_RESULT_ERROR;
    break;
  }
}

/////////// NUMBER OF TOKENS + CURRENT SIZE VERIF ?

static void parse_param(ethPluginProvideParameter_t *msg,
                        context_t *context)
{
  PRINTF("PARSE PARAM\n");
  switch ((parameters)context->param_index)
  {
  case PARAM_OFFERER:
    PRINTF("PARAM_OFFERER\n");
    // copy_address(context->offerer_address, msg->parameter, ADDRESS_LENGTH);
    context->param_index = PARAM_ZONE;
    break;
  case PARAM_ZONE:
    PRINTF("PARAM_ZONE\n");
    context->param_index = PARAM_OFFER_OFFSET;
    break;
  case PARAM_OFFER_OFFSET:
    PRINTF("PARAM_OFFER_OFFSET\n");
    context->param_index = PARAM_CONSIDERATION_OFFSET;
    break;
  case PARAM_CONSIDERATION_OFFSET:
    PRINTF("PARAM_CONSIDERATION_OFFSET\n");
    context->param_index = PARAM_ORDER_TYPE;
    break;
  case PARAM_ORDER_TYPE:
    PRINTF("PARAM_ORDER_TYPE\n");
    context->param_index = PARAM_START_TIME;
    break;
  case PARAM_START_TIME:
    PRINTF("PARAM_START_TIME\n");
    context->param_index = PARAM_END_TIME;
    break;
  case PARAM_END_TIME:
    PRINTF("PARAM_END_TIME\n");
    context->param_index = PARAM_ZONE_HASH;
    break;
  case PARAM_ZONE_HASH:
    PRINTF("PARAM_ZONE_HASH\n");
    context->param_index = PARAM_SALT;
    break;
  case PARAM_SALT:
    PRINTF("PARAM_SALT\n");
    context->param_index = PARAM_CONDUIT_KEY;
    break;
  case PARAM_CONDUIT_KEY:
    PRINTF("PARAM_CONDUIT_KEY\n");
    context->param_index = PARAM_TOTAL_ORIGINAL_CONSIDERATION_ITEMS;
    break;
  case PARAM_TOTAL_ORIGINAL_CONSIDERATION_ITEMS:
    PRINTF("PARAM_TOTAL_ORIGINAL_CONSIDERATION_ITEMS\n");
    context->param_index = PARAM_OFFERS_LEN;
    context->items_index = 0;
    break;
  case PARAM_OFFERS_LEN:
    PRINTF("PARAM_OFFERS_LEN\n");
    context->current_length = U2BE(msg->parameter, PARAMETER_LENGTH - 2);
    if (context->current_length == 0)
    {
      PRINTF("NO OFFERS\n");
      context->param_index = PARAM_CONSIDERATIONS_LEN;
    }
    else
      context->param_index = PARAM_OFFERS;
    break;
  case PARAM_OFFERS:
    PRINTF("PARAM_OFFERS\n");
    PRINTF("OFFER CURRENT_LEN:%d\n", context->current_length);
    parse_offer(msg, context);
    if (context->current_length == 0)
    {
      context->param_index = PARAM_CONSIDERATIONS_LEN;
      context->items_index = 0;
    }
    break;
  case PARAM_CONSIDERATIONS_LEN:
    PRINTF("PARAM_CONSIDERATIONS_LEN\n");
    context->current_length = U2BE(msg->parameter, PARAMETER_LENGTH - 2);
    if (context->current_length == 0)
    {
      PRINTF("NO CONSIDERATIONS\n");
      context->param_index = PARAM_END;
    }
    else
      context->param_index = PARAM_CONSIDERATIONS;
    break;
  case PARAM_CONSIDERATIONS:
    PRINTF("PARAM_CONSIDERATIONS\n");
    PRINTF("CONSIDERATION CURRENT_LEN:%d\n", context->current_length);
    parse_considerations(msg, context);
    if (context->current_length == 0)
    {
      context->param_index = PARAM_END;
      context->items_index = 0;
    }
    break;
  default:
    PRINTF("Param not supported: %d\n", context->param_index);
    msg->result = ETH_PLUGIN_RESULT_ERROR;
    break;
  }
}

static void parse_orders(ethPluginProvideParameter_t *msg,
                         context_t *context)
{
  switch ((orders)context->orders_index)
  {
  case ORDER_PARAMETER_OFFSET:
    PRINTF("ORDER_PARAMETER_OFFSET\n");
    context->orders_index = ORDER_SIGNATURE_OFFSET;
    break;
  case ORDER_SIGNATURE_OFFSET:
    PRINTF("ORDER_SIGNATURE_OFFSET\n");
    context->orders_index = ORDER_PARAMETER;
    break;
  case ORDER_PARAMETER:
    PRINTF("ORDER_PARAMETER\n");
    parse_param(msg, context);
    if (context->param_index == PARAM_END)
    {
      PRINTF("PARAM END\n");
      context->param_index = 0;
      context->orders_index = ORDER_SIGNATURE;
    }
    break;
  case ORDER_SIGNATURE:
    PRINTF("ORDER_SIGNATURE\n"); // If len = 0 what happens ?
    context->current_length = U2BE(msg->parameter, PARAMETER_LENGTH - 2);
    if (context->current_length % 32)
    {
      context->skip++;
    }
    while (context->current_length >= 32)
    {
      context->current_length -= 32;
      context->skip++;
    }
    context->orders_len--;
    context->orders_index = ORDER_PARAMETER_OFFSET;
    break;
  default:
    PRINTF("Param not supported: %d\n", context->param_index);
    msg->result = ETH_PLUGIN_RESULT_ERROR;
    break;
  }
}

static void handle_fulfill_advanced_order(ethPluginProvideParameter_t *msg,
                                          context_t *context)
{
  switch ((fulfill_advanced_order)context->next_param)
  {
  case FADO_OFFSET:
    PRINTF("FADO_OFFSET\n");
    context->next_param = FADO_CRITERIA_RESOLVERS_OFFSET;
    break;
  case FADO_CRITERIA_RESOLVERS_OFFSET:
    PRINTF("FADO_CRITERIA_RESOLVERS_OFFSET\n");
    context->next_param = FADO_FULFILLER_CONDUIT_KEY;
    break;
  case FADO_FULFILLER_CONDUIT_KEY:
    PRINTF("FADO_FULFILLER_CONDUIT_KEY\n");
    context->next_param = FADO_RECIPIENT;
    break;
  case FADO_RECIPIENT:
    PRINTF("FADO_RECIPIENT\n");
    context->next_param = FADO_PARAM_OFFSET;
    break;
  case FADO_PARAM_OFFSET:
    PRINTF("FADO_PARAM_OFFSET\n");
    context->next_param = FADO_NUMERATOR;
    break;
  case FADO_NUMERATOR:
    PRINTF("FADO_NUMERATOR\n");
    context->next_param = FADO_DENOMINATOR;
    break;
  case FADO_DENOMINATOR:
    PRINTF("FADO_DENOMINATOR\n");
    context->next_param = FADO_SIGNATURE_OFFSET;
    break;
  case FADO_SIGNATURE_OFFSET:
    PRINTF("FADO_SIGNATURE_OFFSET\n");
    context->next_param = FADO_EXTRADATA_OFFSET;
    break;
  case FADO_EXTRADATA_OFFSET:
    PRINTF("FADO_EXTRADATA_OFFSET\n");
    context->next_param = FADO_PARAM;
    break;
  case FADO_PARAM:
    PRINTF("FADO_PARAM\n");
    parse_param(msg, context);
    if (context->param_index == PARAM_END)
    {
      PRINTF("PARAM END\n");
      context->param_index = 0;
      context->next_param = FADO_SIGNATURE_LEN;
    }
    break;
  case FADO_SIGNATURE_LEN:
    PRINTF("FADO_CRITERIA_RESOLVERS_OFFSET\n");
    break;
  default:
    PRINTF("Param not supported: %d\n", context->param_index);
    msg->result = ETH_PLUGIN_RESULT_ERROR;
    break;
  }
}

static void handle_fulfill_available_orders(ethPluginProvideParameter_t *msg,
                                            context_t *context)
{
  switch ((fulfill_available_orders)context->next_param)
  {
  case FAO_OFFSET:
    PRINTF("FAO_OFFSET\n");
    context->next_param = FAO_OFFER_FULFILLMENT_OFFSET;
    break;
  case FAO_OFFER_FULFILLMENT_OFFSET:
    PRINTF("FAO_OFFER_FULFILLMENT_OFFSET\n");
    context->next_param = FAO_CONSIDERATION_FULFILLMENT_OFFSET;
    break;
  case FAO_CONSIDERATION_FULFILLMENT_OFFSET:
    PRINTF("FAO_CONSIDERATION_FULFILLMENT_OFFSET\n");
    context->next_param = FAO_FULFILLER_CONDUIT_KEY;
    break;
  case FAO_FULFILLER_CONDUIT_KEY:
    PRINTF("FAO_FULFILLER_CONDUIT_KEY\n");
    context->next_param = FAO_MAXIMUM_FULFILLED;
    break;
  case FAO_MAXIMUM_FULFILLED:
    PRINTF("FAO_MAXIMUM_FULFILLED\n");
    context->next_param = FAO_ORDERS_LEN;
    break;
  case FAO_ORDERS_LEN:
    PRINTF("FAO_ORDERS_LEN\n");
    context->orders_len = U2BE(msg->parameter, PARAMETER_LENGTH - 2);
    context->skip = context->orders_len;
    PRINTF("ORDER_LEN FOUND:%d\n", context->orders_len);
    context->next_param = FAO_ORDERS;
    break;
  case FAO_ORDERS:
    PRINTF("FAO_ORDERS\n");
    parse_orders(msg, context);
    PRINTF("PARSE ORDERS LEN:%d\n", context->orders_len);
    if (context->orders_len == 0)
    {
      PRINTF("END ORDERS\n");
      context->next_param = FAO_FULFILLMEMTS;
    }
    break;
  case FAO_FULFILLMEMTS:
    PRINTF("FAO_FULFILLMEMTS\n");
    break;
  default:
    PRINTF("Param not supported: %d\n", context->param_index);
    msg->result = ETH_PLUGIN_RESULT_ERROR;
    break;
  }
}

static void handle_fullfill_order(ethPluginProvideParameter_t *msg,
                                  context_t *context)
{
  switch ((fulfill_order)context->next_param)
  {
  case FO_OFFSET:
    PRINTF("FO_OFFSET\n");
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
    context->next_param = FO_ORDER_PARAM;
    break;
  case FO_ORDER_PARAM:
    PRINTF("FO_ORDER_PARAM\n");
    parse_param(msg, context);
    if (context->param_index == PARAM_END)
    {
      PRINTF("PARAM END\n");
      context->param_index = 0;
      context->next_param = FO_ORDER_SIGNATURE;
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
  case FULFILL_ADVANCED_ORDER:
    handle_fulfill_advanced_order(msg, context);
    break;
  case FULFILL_AVAILABLE_ORDERS:
    handle_fulfill_available_orders(msg, context);
    break;
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