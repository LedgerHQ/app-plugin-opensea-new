#include "seaport_plugin.h"

static void print_item2(context_t *context)
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
  print_item2(context);
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
  case FULFILL_ADVANCED_ORDER:
  case FULFILL_AVAILABLE_ORDERS:
  case FULFILL_AVAILABLE_ADVANCED_ORDERS:
    break;
  default:
    msg->result = ETH_PLUGIN_RESULT_ERROR;
    break;
  }
  // Determine screens count.
  switch ((selector_t)context->selectorIndex)
  {
  case FULFILL_ORDER:
  case FULFILL_BASIC_ORDER:
  case FULFILL_AVAILABLE_ORDERS:
  case FULFILL_ADVANCED_ORDER:
  case FULFILL_AVAILABLE_ADVANCED_ORDERS:
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
