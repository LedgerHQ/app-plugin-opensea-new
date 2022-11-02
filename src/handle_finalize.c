#include "seaport_plugin.h"

/*
**  Debug
*/

#ifdef DBG_PLUGIN
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
    PRINTF("End context\n");
}
#endif

/*
** Screens Utility
*/

// Return the number of bit raised in screen_array
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

    msg->uiType = ETH_UI_TYPE_GENERIC;

    if (context->booleans & PARSE_ERROR || context->selectorIndex == MATCH_ORDERS ||
        context->selectorIndex == MATCH_ADVANCED_ORDERS) {
        context->screen_array |= PARSE_ERROR_UI;
        msg->numScreens = 1;
        return;
    }

    if (context->booleans & CANT_CALC_AMOUNT) context->screen_array |= PARSE_ERROR_UI;

    switch (context->selectorIndex) {
        case FULFILL_AVAILABLE_ADVANCED_ORDERS:
        case FULFILL_ADVANCED_ORDER:
            if (!ADDRESS_IS_NULL_ADDRESS(context->recipient_address) &&
                memcmp(context->recipient_address, msg->address, ADDRESS_LENGTH)) {
                context->booleans |= IS_BUY4;
                context->screen_array |= BUY_FOR_UI;
            }
        case FULFILL_ORDER:
        case FULFILL_BASIC_ORDER:
        case FULFILL_AVAILABLE_ORDERS:
            context->screen_array |= SEND_UI;
            context->screen_array |= RECEIVE_UI;
            break;
        case INCREMENT_COUNTER:
        case WYVERN_V2_INCREMENT_NONCE:
            context->booleans |= ORDERS;
        case CANCEL:
        case WYVERN_V2_CANCEL_ORDER__:
            context->screen_array |= CANCEL_UI;
            break;
        case WETH_WITHDRAW:
        case WETH_DEPOSIT:
        case POLYGON_BRIDGE_DEPOSIT_ETHER_FOR:
        case ARBITRUM_BRIDGE_DEPOSIT_ETH:
        case OPTIMISM_BRIDGE_DEPOSIT_ETH:
        case UNISWAP_MULTICALL:
            context->screen_array |= ADD_FUNDS_UI;
            break;
        default:
            break;
    }

    // swap tokens when needed:
    switch (context->selectorIndex) {
        case FULFILL_AVAILABLE_ADVANCED_ORDERS:
        case FULFILL_ADVANCED_ORDER:
        case FULFILL_AVAILABLE_ORDERS:
        case FULFILL_ORDER:
            PRINTF("Swapping tokens.\n");
            swap_tokens(context);
        default:
            break;
    }

    // set booleans for fulfillBasicOrder
    if (context->selectorIndex == FULFILL_BASIC_ORDER) {
        switch (context->order_type) {
            case ETH_NFT:
                context->booleans |= ITEM2_IS_NFT;
                context->token1.type = NATIVE;
                context->token2.type = NFT;
                break;
            case ERC20_NFT:
                context->booleans |= ITEM2_IS_NFT;
                context->token1.type = ERC20;
                context->token2.type = NFT;
                break;
            case NFT_ERC20:
                context->booleans |= ITEM1_IS_NFT;
                context->token1.type = NFT;
                context->token2.type = ERC20;
                break;
            default:
                PRINTF("PENZO FINALIZE selectorIndex switch ERROR\n");
        }
    }

    msg->tokenLookup1 = context->token1.address;
    PRINTF("Setting msg->tokenLookup1 to: %.*H\n", ADDRESS_LENGTH, msg->tokenLookup1);

    msg->tokenLookup2 = context->token2.address;
    PRINTF("Setting msg->tokenLookup2 to: %.*H\n", ADDRESS_LENGTH, msg->tokenLookup2);

    msg->numScreens = count_screens(context->screen_array);
    PRINTF("Setting msg->numScreens to: %d\n", msg->numScreens);

#ifdef DBG_PLUGIN
    print_context(context);  // dbg
#endif

    msg->result = ETH_PLUGIN_RESULT_OK;
}
