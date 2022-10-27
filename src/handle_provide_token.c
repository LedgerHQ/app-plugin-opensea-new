#include "seaport_plugin.h"

// EDIT THIS: Adapt this function to your needs! Remember, the information for
// tokens are held in `msg->item1` and `msg->item2`. If those pointers are
// `NULL`, this means the ethereum app didn't find any info regarding the
// requested tokens!

void handle_provide_token(void *parameters) {
    ethPluginProvideInfo_t *msg = (ethPluginProvideInfo_t *) parameters;
    context_t *context = (context_t *) msg->pluginContext;

    if (msg->item1) context->booleans |= ITEM1_FOUND;
    // check if not ETH address
    else if (!ADDRESS_IS_NULL_ADDRESS(context->token1.address)) {
        context->screen_array |= SEND_UI_ERR;
        msg->additionalScreens++;
    }

    if (msg->item2) context->booleans |= ITEM2_FOUND;
    // check if not ETH address
    else if (!ADDRESS_IS_NULL_ADDRESS(context->token2.address)) {
        context->screen_array |= RECEIVE_UI_ERR;
        msg->additionalScreens++;
    }

    PRINTF("1GPIRIOU txvalue:\t%.*H\n", INT256_LENGTH, msg->pluginSharedRO->txContent->value.value);
    if (context->selectorIndex == WETH_WITHDRAW) {
        // ticker == WETH;
    }
    switch (context->selectorIndex) {
        case WETH_DEPOSIT:
        case POLYGON_BRIDGE_DEPOSIT_ETH:
        case ARBITRUM_BRIDGE_DEPOSIT_ETH:
        case OPTIMISM_BRIDGE_DEPOSIT_ETH:
            // TODO: value is big endian = revert and store in token1.amount
            break;
        default:
            break;
    }
    PRINTF("2GPIRIOU token1.amount:\t%.*H\n", INT256_LENGTH, context->token1.amount);
    PRINTF("3GPIRIOU txvalue:\t%.*H\n", INT256_LENGTH, msg->pluginSharedRO->txContent->value.value);

    msg->result = ETH_PLUGIN_RESULT_OK;
}