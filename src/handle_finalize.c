#include "seaport_plugin.h"

static void print_context(context_t *context)
{
  PRINTF("__Print context:\n");
  // PRINTF("number_of_tokens:\t%d\n", context->number_of_tokens);
  PRINTF("number_of_nfts:\t%d\n", context->number_of_nfts);
  // PRINTF("number_of_offers:\t%d\n", context->offers_len);
  //  PRINTF("basic_order_type:\t%d\n", context->basic_order_type);
  PRINTF("token1_address:\t%.*H\n", ADDRESS_LENGTH, context->token1_address);
  PRINTF("token1_amount:\t%.*H\n", INT256_LENGTH, context->token1_amount);
  PRINTF("token2_address:\t%.*H\n", ADDRESS_LENGTH, context->token2_address);
  PRINTF("token2_amount:\t%.*H\n", INT256_LENGTH, context->token2_amount);
  // PRINTF("several_collections:%d\n", INT256_LENGTH, context->token2_amount);
  PRINTF("sale_side:%d\n", context->sale_side);
  PRINTF("offer_item_type:%d\n", context->offer_item_type);
  PRINTF("consideration_item_type:%d\n", context->consideration_item_type);
  PRINTF("End context\n");
}

void handle_finalize(void *parameters)
{
  ethPluginFinalize_t *msg = (ethPluginFinalize_t *)parameters;
  context_t *context = (context_t *)msg->pluginContext;

  // set default decimals
  context->token1_decimals = DEFAULT_DECIMAL;
  // set default numScreens
  msg->numScreens = 2;

  switch (context->selectorIndex)
  {
  case FULFILL_BASIC_ORDER:
    if (context->basic_order_type == NFT_ERC20)
    {
      // does not work if number_of_tokens > 256.
      context->number_of_tokens = U2BE(context->token1_amount, INT256_LENGTH - 2);
    }
    else if (context->basic_order_type == ETH_NFT ||
             context->basic_order_type == ERC20_NFT)
    {
      // does not work if number_of_tokens > 256.
      context->number_of_tokens = U2BE(context->token2_amount, INT256_LENGTH - 2);
    }
    break;
  case FULFILL_ORDER:
    break;
  default:
    msg->result = ETH_PLUGIN_RESULT_ERROR;
    break;
  }
  // Determine screens count.
  switch ((selector_t)context->selectorIndex)
  {
  case FULFILL_BASIC_ORDER:
  case FULFILL_ORDER:
    msg->numScreens = 2;
    break;
  default:
    PRINTF("Param not supported: %d\n", context->next_param);
    msg->result = ETH_PLUGIN_RESULT_ERROR;
    return;
  }

  PRINTF("Setting tokenLookup1 to: %.*H\n", ADDRESS_LENGTH,
         context->token1_address);
  msg->tokenLookup1 = context->token1_address;

  PRINTF("Setting tokenLookup2 to: %.*H\n", ADDRESS_LENGTH,
         context->token2_address);
  msg->tokenLookup2 = context->token2_address;

  PRINTF("Booleans:\n");
  PRINTF("BOOL1  %d\n", context->booleans & BOOL1);

  /* print readable amount */
  // uint8_t buf[INT256_LENGTH] = {0};
  // amountToString(context->token1_amount, INT256_LENGTH, 18, "OSEF ", buf,
  // 78); PRINTF("buf amountToString: %s \n", buf);

  print_context(context);

  msg->uiType = ETH_UI_TYPE_GENERIC;
  msg->result = ETH_PLUGIN_RESULT_OK;
}
