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
    context->order_type =
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
    if (context->order_type != NFT_ERC20)
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

static void parse_offer(ethPluginProvideParameter_t *msg, context_t *context)
{
  switch ((fulfill_order_offer)context->enum_param)
  {
  case FO_ORDER_PARAM_OFFER_LEN_ENUM:
    PRINTF("FO_ORDER_PARAM_OFFER_LEN\n");
    context->current_length = U2BE(msg->parameter, PARAMETER_LENGTH - 2);
    PRINTF("CURRENT_LENGTH:%d\n", context->current_length);
    context->number_of_nfts = U2BE(msg->parameter, PARAMETER_LENGTH - 2);
    if (context->current_length == 0)
      context->enum_param = FO_ORDER_PARAM_CONSIDERATION_LEN;
    else
      context->enum_param = FO_ORDER_PARAM_OFFER_ITEM_TYPE;
    PRINTF("FO_ORDER_PARAM_OFFER_LEN\n");
    break;
  case FO_ORDER_PARAM_OFFER_ITEM_TYPE:
    PRINTF("FO_ORDER_PARAM_OFFER_ITEM_TYPE\n");
    context->enum_param = FO_ORDER_PARAM_OFFER_TOKEN;
    break;
  case FO_ORDER_PARAM_OFFER_TOKEN:
    PRINTF("FO_ORDER_PARAM_OFFER_ITEM_TOKEN\n");
    if (context->current_length == context->number_of_nfts)
      copy_address(context->token1_address, msg->parameter, ADDRESS_LENGTH);
    if (memcmp(context->token1_address, msg->parameter, ADDRESS_LENGTH))
      context->several_collections++;
    context->enum_param = FO_ORDER_PARAM_OFFER_IDENTIFIER;
    break;
  case FO_ORDER_PARAM_OFFER_IDENTIFIER:
    PRINTF("FO_ORDER_PARAM_OFFER_IDENTIFIER\n");
    context->enum_param = FO_ORDER_PARAM_OFFER_START_AMOUNT;
    break;
  case FO_ORDER_PARAM_OFFER_START_AMOUNT:
    PRINTF("FO_ORDER_PARAM_OFFER_START_AMOUNT\n");
    context->enum_param = FO_ORDER_PARAM_OFFER_END_AMOUNT;
    break;
  case FO_ORDER_PARAM_OFFER_END_AMOUNT:
    PRINTF("FO_ORDER_PARAM_OFFER_END_AMOUNT\n");
    if (context->current_length > 0)
    {
      context->current_length--;
      PRINTF("LOOP CURRENT_LENGTH:%d\n", context->current_length);
      context->enum_param = FO_ORDER_PARAM_OFFER_ITEM_TYPE;
    }
    else
    {
      context->next_param = FO_ORDER_PARAM_CONSIDERATION_LEN;
      context->enum_param = 0;
    }
    break;
  default:
    PRINTF("Param not supported: %d\n", context->next_param);
    msg->result = ETH_PLUGIN_RESULT_ERROR;
    break;
  }
}

static void parse_consideration(ethPluginProvideParameter_t *msg, context_t *context)
{
  switch ((fulfill_order_consideration)context->enum_param)
  {
  case FO_ORDER_PARAM_CONSIDERATION_LEN_ENUM:
    PRINTF("FO_ORDER_PARAM_CONSIDERATION_LEN\n");
    context->current_length = U2BE(msg->parameter, PARAMETER_LENGTH - 2);
    PRINTF("CURRENT_LENGTH:%d\n", context->current_length);
    context->number_of_tokens = U2BE(msg->parameter, PARAMETER_LENGTH - 2);
    if (context->current_length == 0)
      context->enum_param = FO_ORDER_SIGNATURE;
    else
      context->enum_param = FO_ORDER_PARAM_CONSIDERATION_TOKEN;
    break;
  case FO_ORDER_PARAM_CONSIDERATION_TOKEN:
    PRINTF("FO_ORDER_PARAM_CONSIDERATION_TOKEN\n");
    if (memcmp(context->token2_address, msg->parameter, PARAMETER_LENGTH))
    {
      copy_address(context->token2_address, msg->parameter, PARAMETER_LENGTH);
      context->several_collections++;
    }
    context->enum_param = FO_ORDER_PARAM_CONSIDERATION_IDENTIFIER;
    break;
  case FO_ORDER_PARAM_CONSIDERATION_IDENTIFIER:
    PRINTF("FO_ORDER_PARAM_CONSIDERATION_IDENTIFIER\n");
    context->enum_param = FO_ORDER_PARAM_CONSIDERATION_START_AMOUNT;
    break;
  case FO_ORDER_PARAM_CONSIDERATION_START_AMOUNT:
    PRINTF("FO_ORDER_PARAM_CONSIDERATION_START_AMOUNT\n");
    context->enum_param = FO_ORDER_PARAM_CONSIDERATION_END_AMOUNT;
    break;
  case FO_ORDER_PARAM_CONSIDERATION_END_AMOUNT:
    PRINTF("FO_ORDER_PARAM_CONSIDERATION_END_AMOUNT\n");
    uint8_t buf_amount[INT256_LENGTH] = {0};
    copy_parameter(buf_amount, msg->parameter, PARAMETER_LENGTH);
    add_uint256(context->token1_amount, buf_amount);
    context->enum_param = FO_ORDER_PARAM_CONSIDERATION_RECIPIENT;
    break;
  case FO_ORDER_PARAM_CONSIDERATION_RECIPIENT:
    PRINTF("FO_ORDER_PARAM_CONSIDERATION_RECIPIENT\n");
    if (context->current_length > 0)
    {
      PRINTF("LOOP CURRENT_LENGTH:%d\n", context->current_length);
      context->current_length--;
      context->enum_param = FO_ORDER_PARAM_CONSIDERATION_TOKEN;
    }
    else
      context->next_param = FO_ORDER_SIGNATURE;
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
    PRINTF("FO_ORDER_PARAM_CONDUIT_KEU\n");
    context->next_param = FO_ORDER_PARAM_TOTAL_ORIGINAL_CONSIDERATION_ITEMS;
    break;
  case FO_ORDER_PARAM_TOTAL_ORIGINAL_CONSIDERATION_ITEMS:
    PRINTF("FO_ORDER_PARAM_TOTAL_ORIGINAL_CONSIDERATION_ITEMS\n");
    context->next_param = FO_ORDER_PARAM_OFFER_LEN;
    context->enum_param = 0;
    break;
  case FO_ORDER_PARAM_OFFER_LEN:
    PRINTF("FO_ORDER_PARAM_OFFER_LEN\n");
    parse_offer(msg, context);
    break;
  case FO_ORDER_PARAM_CONSIDERATION_LEN:
    PRINTF("FO_ORDER_PARAM_CONSIDERATION_LEN\n");
    parse_consideration(msg, context);
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
