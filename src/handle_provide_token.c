#include "opensea_plugin.h"

// EDIT THIS: Adapt this function to your needs! Remember, the information for
// tokens are held in `msg->item1` and `msg->item2`. If those pointers are
// `NULL`, this means the ethereum app didn't find any info regarding the
// requested tokens!

void handle_provide_token(ethPluginProvideInfo_t *msg) {
    context_t *context = (context_t *) msg->pluginContext;

    PRINTF("PROVIDE_TOKEN token1.address: %.*H\n", ADDRESS_LENGTH, context->token1.address);
    PRINTF("PROVIDE_TOKEN token2.address: %.*H\n", ADDRESS_LENGTH, context->token2.address);
    if (!memcmp(context->token2.address, STOREFRONT_LAZYMINTER, ADDRESS_LENGTH)) {
        context->transaction_info |= IS_STOREFRONT;
    }

    if (msg->item1) context->transaction_info |= ITEM1_FOUND;
    // check if not ETH address
    else if (!ADDRESS_IS_NULL_ADDRESS(context->token1.address)) {
        context->screen_array |= SEND_UI_ERR;
        msg->additionalScreens++;
    } else {
        PRINTF("ITEM1 IS NATIVE!\n");
    }
    // Check if address is LAZYMINTER_PROXY address for OpenSea Shared Storefront custom
    // collections.
    if (msg->item2 || (!msg->item2 && context->transaction_info & IS_STOREFRONT)) {
        context->transaction_info |= ITEM2_FOUND;
    }
    // check if not ETH address

    else if (!ADDRESS_IS_NULL_ADDRESS(context->token2.address)) {
        context->screen_array |= RECEIVE_UI_ERR;
        msg->additionalScreens++;
    } else {
        PRINTF("ITEM2 IS NATIVE!\n");
    }

    if (context->token1.type == MULTIPLE_NFTS) {
        context->screen_array |= SEND_UI_ERR;
    }

    if (context->token2.type == MULTIPLE_NFTS) {
        context->screen_array |= RECEIVE_UI_ERR;
    }

    msg->result = ETH_PLUGIN_RESULT_OK;
}
