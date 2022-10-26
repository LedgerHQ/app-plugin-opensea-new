#include "seaport_plugin.h"
#include "text.h"

// Sets the first screen to display.
void handle_query_contract_id(void *parameters) {
    ethQueryContractID_t *msg = (ethQueryContractID_t *) parameters;
    context_t *context = (context_t *) msg->pluginContext;
    // msg->name will be the upper sentence displayed on the screen.
    // msg->version will be the lower sentence displayed on the screen.

    // For the first screen, display the plugin name.
    strlcpy(msg->name, PLUGIN_NAME, msg->nameLength);

    // swap tokens when needed:
    switch (context->selectorIndex) {
        case FULFILL_AVAILABLE_ORDERS:
            PRINTF("ID SELL_SIDE is_accept? %d\n", context->booleans & IS_ACCEPT);
            if (context->token1.type == NFT) swap_tokens(context);
            break;
        case FULFILL_BASIC_ORDER:
            break;
        default:
            break;
    }

    // Get selector according screen.
    switch (context->selectorIndex) {
        case FULFILL_BASIC_ORDER:
            PRINTF("PENZO (basic_order_type)context->order_type: %d\n", context->order_type);
        case FULFILL_AVAILABLE_ORDERS:
            strlcpy(msg->version, "exchange", msg->versionLength);
            break;
        default:
            PRINTF("Selector index: %d not supported\n", context->selectorIndex);
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            return;
    }
    msg->result = ETH_PLUGIN_RESULT_OK;
}
