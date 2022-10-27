#include "seaport_plugin.h"

static void print_item2(context_t *context) {
    if (context->token1.type == ITEM_TYPE_NONE)
        PRINTF("OFFER ITEM TYPE FOUND: NONE\n", context->token1.type);

    if (context->token1.type == ITEM_TYPE_NATIVE)
        PRINTF("OFFER ITEM TYPE FOUND: NATIVE\n", context->token1.type);

    if (context->token1.type == ITEM_TYPE_ERC20)
        PRINTF("OFFER ITEM TYPE FOUND: ERC20\n", context->token1.type);

    if (context->token1.type == ITEM_TYPE_NFT)
        PRINTF("OFFER ITEM TYPE FOUND: NFT\n", context->token1.type);

    if (context->token1.type == ITEM_TYPE_MULTIPLE_NFTS)
        PRINTF("OFFER ITEM TYPE FOUND: MULTIPLE NFTS\n", context->token1.type);

    if (context->token1.type == ITEM_TYPE_MULTIPLE_ERC20S)
        PRINTF("OFFER ITEM TYPE FOUND: MULTIPLE_ERC20S\n", context->token1.type);

    if (context->token1.type == ITEM_TYPE_MIXED_TYPES)
        PRINTF("OFFER ITEM TYPE FOUND: MIXED TYPES\n", context->token1.type);

    // if (context->token2.type == ITEM_TYPE_NONE)
    //    PRINTF("CONSIDERATION ITEM TYPE FOUND: NONE\n", context->token2.type);

    // if (context->token2.type == ITEM_TYPE_NATIVE)
    //    PRINTF("CONSIDERATION ITEM TYPE FOUND: NATIVE\n", context->token2.type);

    // if (context->token2.type == ITEM_TYPE_ERC20)
    //    PRINTF("CONSIDERATION ITEM TYPE FOUND: ERC20\n", context->token2.type);

    // if (context->token2.type == ITEM_TYPE_NFT)
    //    PRINTF("CONSIDERATION ITEM TYPE FOUND: NFT\n", context->token2.type);

    // if (context->token2.type == ITEM_TYPE_NFT)
    //    PRINTF("CONSIDERATION ITEM TYPE FOUND: MULTIPLE NFTS\n",
    //    context->token2.type);

    // if (context->token2.type == ITEM_TYPE_MULTIPLE_ERC20S)
    //    PRINTF("CONSIDERATION ITEM TYPE FOUND: MULTIPLE_ERC20S\n",
    //           context->token2.type);

    // if (context->token2.type == ITEM_TYPE_MULTIPLE_ERC20S)
    //    PRINTF("CONSIDERATION ITEM TYPE FOUND: MIXED TYPES\n", context->token2.type);
    (context->booleans & IS_ACCEPT) ? PRINTF("ACCEPT_OFFER\n") : PRINTF("BUY_NOW\n");
}

static void print_context(context_t *context) {
    PRINTF("__Print context:\n");
    // PRINTF("number_of_tokens:\t%d\n", context->number_of_tokens);
    PRINTF("number_of_nfts:\t%d\n", context->number_of_nfts);
    // PRINTF("number_of_offers:\t%d\n", context->offers_len);
    //  PRINTF("basic_order_type:\t%d\n", context->basic_order_type);
    PRINTF("token1.address:\t%.*H\n", ADDRESS_LENGTH, context->token1.address);
    PRINTF("token1.amount:\t%.*H\n", INT256_LENGTH, context->token1.amount);
    PRINTF("token2.address:\t%.*H\n", ADDRESS_LENGTH, context->token2.address);
    PRINTF("token2.amount:\t%.*H\n", INT256_LENGTH, context->token2.amount);
    PRINTF("token1.type:%d\n", context->token1.type);
    PRINTF("token2.type:%d\n", context->token2.type);
    print_item2(context);
    PRINTF("End context\n");
}

/*
** Screens Utility
*/

static uint8_t count_screens(uint8_t screen_array) {
    uint8_t total = 0;
    uint8_t scout = 1;
    for (uint8_t i = 0; i < 8; i++) {
        if (scout & screen_array) total++;
        scout <<= 1;
    }
    return total;
}

/*
** handle_finalize()
*/

void handle_finalize(void *parameters) {
    ethPluginFinalize_t *msg = (ethPluginFinalize_t *) parameters;
    context_t *context = (context_t *) msg->pluginContext;

    context->screen_array |= SEND_UI;
    switch (context->selectorIndex) {
        case FULFILL_ORDER:
        case FULFILL_BASIC_ORDER:
        case FULFILL_AVAILABLE_ORDERS:
        case FULFILL_ADVANCED_ORDER:
        case FULFILL_AVAILABLE_ADVANCED_ORDERS:
        case MATCH_ORDERS:
        case MATCH_ADVANCED_ORDERS:
            context->screen_array |= RECEIVE_UI;
            break;
        case CANCEL:
        case INCREMENT_COUNTER:
            context->screen_array &= SEND_UI;
            context->screen_array |= CANCEL_UI;
            break;
        case WETH_WITHDRAW:
        case WETH_DEPOSIT:
        case POLYGON_BRIDGE_DEPOSIT_ETH:
        case ARBITRUM_BRIDGE_DEPOSIT_ETH:
        case OPTIMISM_BRIDGE_DEPOSIT_ETH:
            break;
        default:
            break;
    }

    // set IS_ETH if one of the addresses is 0x0000...
    if (!memcmp(context->token1.address, NULL_ADDRESS, ADDRESS_LENGTH) ||
        !memcmp(context->token2.address, NULL_ADDRESS, ADDRESS_LENGTH))
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

    PRINTF("Setting tokenLookup1 to: %.*H\n", ADDRESS_LENGTH, context->token1.address);
    msg->tokenLookup1 = context->token1.address;

    PRINTF("Setting tokenLookup2 to: %.*H\n", ADDRESS_LENGTH, context->token2.address);
    msg->tokenLookup2 = context->token2.address;

    msg->numScreens = count_screens(context->screen_array);
    print_context(context);  // dbg

    msg->uiType = ETH_UI_TYPE_GENERIC;
    msg->result = ETH_PLUGIN_RESULT_OK;
}
