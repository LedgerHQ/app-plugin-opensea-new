#include "seaport_plugin.h"
#include "text.h"

#ifdef DBG_PLUGIN
static void print_booleans(context_t *context) {
    PRINTF("Booleans:\n");
    PRINTF(
        "\
%d ERROR\n\
%d IS_BUY4\n\
%d IS_ACCEPT\n\
%d ITEM1_IS_NFT\n\
%d ITEM2_IS_NFT\n\
%d ITEM1_FOUND\n\
%d ITEM2_FOUND\n\
%d IS_ETH\n\
%d CANT_CALC_AMOUNT\n\
",
        (context->booleans & ERROR) ? 1 : 0,
        (context->booleans & IS_BUY4) ? 1 : 0,
        (context->booleans & IS_ACCEPT) ? 1 : 0,
        (context->booleans & ITEM1_IS_NFT) ? 1 : 0,
        (context->booleans & ITEM2_IS_NFT) ? 1 : 0,
        (context->booleans & ITEM1_FOUND) ? 1 : 0,
        (context->booleans & ITEM2_FOUND) ? 1 : 0,
        (context->booleans & IS_ETH) ? 1 : 0,
        (context->booleans & CANT_CALC_AMOUNT) ? 1 : 0);
}
#endif

// Sets the first screen to display.
void handle_query_contract_id(void *parameters) {
    ethQueryContractID_t *msg = (ethQueryContractID_t *) parameters;
    context_t *context = (context_t *) msg->pluginContext;
    // msg->name will be the upper sentence displayed on the screen.
    // msg->version will be the lower sentence displayed on the screen.

#ifdef DBG_PLUGIN
    print_booleans(context);
#endif

    // Reset screens management state if user come back to ID screen.
    context->previous_screen_index = 0;
    context->plugin_screen_index = 0;

    // For the first screen, display the plugin name.
    strlcpy(msg->name, PLUGIN_NAME, msg->nameLength);

    // Get selector according screen.
    switch (context->selectorIndex) {
        case FULFILL_ORDER:
        case FULFILL_BASIC_ORDER:
        case FULFILL_AVAILABLE_ADVANCED_ORDERS:
        case MATCH_ORDERS:
        case MATCH_ADVANCED_ORDERS:
        case CANCEL:
        case INCREMENT_COUNTER:
        case WETH_DEPOSIT:
        case WETH_WITHDRAW:
            PRINTF("PENZO (basic_order_type)context->order_type: %d\n", context->order_type);
        case FULFILL_AVAILABLE_ORDERS:
        case FULFILL_ADVANCED_ORDER:
            strlcpy(msg->version, "exchange", msg->versionLength);
            break;
        case POLYGON_BRIDGE_DEPOSIT_ETH:
        case ARBITRUM_BRIDGE_DEPOSIT_ETH:
        case OPTIMISM_BRIDGE_DEPOSIT_ETH:
            strlcpy(msg->version, "Bridge", msg->versionLength);
            break;
        default:
            PRINTF("Selector index: %d not supported\n", context->selectorIndex);
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            return;
    }
    msg->result = ETH_PLUGIN_RESULT_OK;
}
