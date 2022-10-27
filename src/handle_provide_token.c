#include "seaport_plugin.h"

// EDIT THIS: Adapt this function to your needs! Remember, the information for
// tokens are held in `msg->item1` and `msg->item2`. If those pointers are
// `NULL`, this means the ethereum app didn't find any info regarding the
// requested tokens!

void handle_provide_token(void *parameters) {
    ethPluginProvideInfo_t *msg = (ethPluginProvideInfo_t *) parameters;
    context_t *context = (context_t *) msg->pluginContext;

    PRINTF("PROVIDE_TOKEN token1.address: %.*H\n", ADDRESS_LENGTH, context->token1.address);
    PRINTF("PROVIDE_TOKEN token2.address: %.*H\n", ADDRESS_LENGTH, context->token2.address);

    if (msg->item1) context->booleans |= ITEM1_FOUND;
    // check if not ETH address
    else if (!ADDRESS_IS_NULL_ADDRESS(context->token1.address)) {
        context->screen_array |= SEND_UI_ERR;
        msg->additionalScreens++;
    } else
        PRINTF("\n\n\tITEM1 IS ETH!\n\n");

    if (msg->item2) context->booleans |= ITEM2_FOUND;
    // check if not ETH address
    else if (!ADDRESS_IS_NULL_ADDRESS(context->token2.address)) {
        context->screen_array |= RECEIVE_UI_ERR;
        msg->additionalScreens++;
    } else
        PRINTF("\n\n\tITEM2 IS ETH!\n\n");

    msg->result = ETH_PLUGIN_RESULT_OK;
}