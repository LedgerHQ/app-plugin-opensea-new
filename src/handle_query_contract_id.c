#include "opensea_plugin.h"

#ifdef DBG_PLUGIN
static void print_booleans(context_t *context) {
    PRINTF("Booleans:\n");
    PRINTF(
        "\
%d IS_BUY4\n\
%d IS_ACCEPT\n\
%d ITEM1_IS_NFT\n\
%d ITEM2_IS_NFT\n\
%d ITEM1_FOUND\n\
%d ITEM2_FOUND\n\
%d CANT_CALC_AMOUNT\n\
",
        (context->transaction_info & IS_BUY4) ? 1 : 0,
        (context->transaction_info & IS_ACCEPT) ? 1 : 0,
        (context->transaction_info & ITEM1_IS_NFT) ? 1 : 0,
        (context->transaction_info & ITEM2_IS_NFT) ? 1 : 0,
        (context->transaction_info & ITEM1_FOUND) ? 1 : 0,
        (context->transaction_info & ITEM2_FOUND) ? 1 : 0,
        (context->transaction_info & CANT_CALC_AMOUNT) ? 1 : 0);
}
#endif

// Sets the first screen to display.
void handle_query_contract_id(ethQueryContractID_t *msg) {
    context_t *context = (context_t *) msg->pluginContext;
    // msg->name will be the upper sentence displayed on the screen.
    // msg->version will be the lower sentence displayed on the screen.

#ifdef DBG_PLUGIN
    print_booleans(context);
#endif

    // Reset screens management state if user come back to ID screen.
    context->prev_screenIndex = 0;
    context->screen_probe = 0;

    // For the first screen, display the plugin name.
    strlcpy(msg->name, PLUGIN_NAME, msg->nameLength);

    // Get selector according screen.
    switch (context->selectorIndex) {
        case FULFILL_ORDER:
        case FULFILL_BASIC_ORDER:
        case FULFILL_AVAILABLE_ADVANCED_ORDERS:
        case CANCEL:
        case INCREMENT_COUNTER:
        case WYVERN_V2_CANCEL_ORDER__:
        case WYVERN_V2_INCREMENT_NONCE:
        case WETH_DEPOSIT:
        case WETH_WITHDRAW:
        case FULFILL_AVAILABLE_ORDERS:
        case FULFILL_ADVANCED_ORDER:
        case UNISWAP_MULTICALL:
            strlcpy(msg->version, "Exchange", msg->versionLength);
            break;
        case POLYGON_BRIDGE_DEPOSIT_ETHER_FOR:
        case ARBITRUM_BRIDGE_DEPOSIT_ETH:
            strlcpy(msg->version, "Bridge", msg->versionLength);
            break;
        default:
            PRINTF("Selector index: %d not supported\n", context->selectorIndex);
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            return;
    }
    msg->result = ETH_PLUGIN_RESULT_OK;
}
