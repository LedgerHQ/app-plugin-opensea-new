#include "seaport_plugin.h"

/*
** Debug
*/

static void print_context(context_t *context) {
  PRINTF("__Print context:\n");
  PRINTF("number_of_tokens:\t%d\n", context->number_of_tokens);
  PRINTF("order_type:\t%d\n", context->order_type);
  PRINTF("token1_address:\t%.*H\n", ADDRESS_LENGTH, context->token1_address);
  PRINTF("token1_amount:\t%.*H\n", INT256_LENGTH, context->token1_amount);
  PRINTF("token2_address:\t%.*H\n", ADDRESS_LENGTH, context->token2_address);
  PRINTF("token2_amount:\t%.*H\n", INT256_LENGTH, context->token2_amount);
  PRINTF("End context\n");
}

/*
** Screens Utility
*/

static uint8_t count_screens(uint8_t screen_array) {
  uint8_t total = 0;
  uint8_t scout = 1;
  for (uint8_t i = 0; i < 8; i++) {
    if (scout & screen_array)
      total++;
    scout <<= 1;
  }
  return total;
}

/*
** handle_finalize()
*/

void handle_finalize(void *parameters) {
  ethPluginFinalize_t *msg = (ethPluginFinalize_t *)parameters;
  context_t *context = (context_t *)msg->pluginContext;

  // set default decimals
  context->token1_decimals = DEFAULT_DECIMAL;
  context->screen_array |= SEND_UI;
  context->screen_array |= RECEIVE_UI;

  // set IS_ETH if one of the addresses is 0x0000...
  if (!memcmp(context->token1_address, NULL_ADDRESS, ADDRESS_LENGTH) ||
      !memcmp(context->token2_address, NULL_ADDRESS, ADDRESS_LENGTH))
    context->booleans |= IS_ETH;

  // set booleans for fulfillBasicOrder
  if (context->selectorIndex == FULFILL_BASIC_ORDER) {
    PRINTF("PENZO FINALIZE IM IN!\n");
    switch (context->order_type) {
    case ETH_NFT:
      context->booleans |= ITEM2_IS_NFT;
      break;
    case ERC20_NFT:
      context->booleans |= ITEM2_IS_NFT;
      break;
    case NFT_ERC20:
      context->booleans |= ITEM1_IS_NFT;
      break;
    default:
      PRINTF("PENZO FINALIZE selectorIndex switch ERROR\n");
    }
  }

  PRINTF("Setting tokenLookup1 to: %.*H\n", ADDRESS_LENGTH,
         context->token1_address);
  msg->tokenLookup1 = context->token1_address;

  PRINTF("Setting tokenLookup2 to: %.*H\n", ADDRESS_LENGTH,
         context->token2_address);
  msg->tokenLookup2 = context->token2_address;

  msg->numScreens = count_screens(context->screen_array);
  print_context(context); // dbg

  msg->uiType = ETH_UI_TYPE_GENERIC;
  msg->result = ETH_PLUGIN_RESULT_OK;
}
