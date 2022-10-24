#include "seaport_plugin.h"

static uint8_t get_basic_order_type(ethPluginProvideParameter_t *msg,
                                    uint8_t basic_order_type_sol) {
    uint8_t type = 0;
    switch (basic_order_type_sol) {
        case 0:
        case 1:
        case 2:
        case 3:
        case 4:
        case 5:
        case 6:
        case 7:
            type = ETH_NFT;
            break;
        case 8:
        case 9:
        case 10:
        case 11:
        case 12:
        case 13:
        case 14:
        case 15:
            type = ERC20_NFT;
            break;
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
            type = NFT_ERC20;
            break;
        default:
            PRINTF("undefined basic_order_type_sol\n");
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
    return type;
}

static void handle_fulfill_basic_order(ethPluginProvideParameter_t *msg, context_t *context) {
    switch ((fulfill_basic_order_parameter) context->next_param) {
        case FBO__OFFSET_BASIC_ORDER_PARAM:
            PRINTF("FBO__OFFSET_BASIC_ORDER_PARAM\n");
            context->next_param = FBO__CONSIDERATION_TOKEN;
            break;
        case FBO__CONSIDERATION_TOKEN:
            PRINTF("FBO__CONSIDERATION_TOKEN\n");
            copy_address(context->token1_address, msg->parameter, ADDRESS_LENGTH);
            context->next_param = FBO__CONSIDERATION_IDENTIFIER;
            break;
        case FBO__CONSIDERATION_IDENTIFIER:
            PRINTF("FBO__CONSIDERATION_IDENTIFIER\n");
            context->next_param = FBO__CONSIDERATION_AMOUNT;
            break;
        case FBO__CONSIDERATION_AMOUNT:
            PRINTF("FBO__CONSIDERATION_AMOUNT\n");
            copy_parameter(context->token1_amount, msg->parameter, PARAMETER_LENGTH);
            context->next_param = FBO__OFFERER;
            break;
        case FBO__OFFERER:
            PRINTF("FBO__OFFERER\n");
            context->skip = 1;
            context->next_param = FBO__OFFER_TOKEN;
            break;
        case FBO__OFFER_TOKEN:
            PRINTF("FBO__OFFER_TOKEN\n");
            copy_address(context->token2_address, msg->parameter, PARAMETER_LENGTH);
            context->next_param = FBO__OFFER_IDENTIFIER;
            break;
        case FBO__OFFER_IDENTIFIER:
            PRINTF("FBO__OFFER_IDENTIFIER\n");
            // copy_parameter(context->nft_id, msg->parameter, PARAMETER_LENGTH);
            context->next_param = FBO__OFFER_AMOUNT;
            break;
        case FBO__OFFER_AMOUNT:
            PRINTF("FBO__OFFER_AMOUNT\n");
            copy_parameter(context->token2_amount, msg->parameter, PARAMETER_LENGTH);
            context->next_param = FBO__BASIC_ORDER_TYPE;
            break;
        case FBO__BASIC_ORDER_TYPE:
            PRINTF("FBO__BASIC_ORDER_TYPE\n");
            context->order_type =
                get_basic_order_type(msg, U2BE(msg->parameter, PARAMETER_LENGTH - 2));
            context->skip = 9;
            context->next_param = FBO__LEN_ADDITIONAL_RECIPIENTS;
            break;
        case FBO__LEN_ADDITIONAL_RECIPIENTS:
            PRINTF("FBO__LEN_ADDITIONAL_RECIPIENTS\n");
            context->current_length = U2BE(msg->parameter, PARAMETER_LENGTH - 2);
            context->next_param = FBO__ADDITIONAL_AMOUNT;
            break;
        case FBO__ADDITIONAL_AMOUNT:
            PRINTF("FBO__ADDITIONAL_AMOUNT\n");
            uint8_t buf_amount[INT256_LENGTH] = {0};
            if (context->order_type == ERC20_NFT || context->order_type == ETH_NFT) {
                copy_parameter(buf_amount, msg->parameter, PARAMETER_LENGTH);
                if (add_uint256(context->token1_amount, buf_amount)) {
                    PRINTF("ERROR: uint256 overflow error.\n");
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                }
            } else {  // is NFT_ERC20
                copy_parameter(buf_amount, msg->parameter, PARAMETER_LENGTH);
                if (sub_uint256(context->token2_amount, buf_amount)) {
                    PRINTF("ERROR: uint256 overflow error.\n");
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                }
            }
            if (0 == --context->current_length) context->next_param = FBO__LEN_SIGNATURE;
            context->skip = 1;
            break;
        case FBO__LEN_SIGNATURE:
            PRINTF("FBO__LEN_SIGNATURE\n");
            break;
        default:
            PRINTF("Param not supported: %d\n", context->next_param);
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
}

void handle_provide_parameter(void *parameters) {
    ethPluginProvideParameter_t *msg = (ethPluginProvideParameter_t *) parameters;
    context_t *context = (context_t *) msg->pluginContext;

    // Print Parameter
    PRINTF(
        "___\nplugin provide parameter: offset %d\nBytes: \033[0;31m %.*H "
        "\033[0m \n",
        msg->parameterOffset,
        PARAMETER_LENGTH,
        msg->parameter);

    msg->result = ETH_PLUGIN_RESULT_OK;

    if (context->skip > 0) {
        PRINTF("SKIPPED\n");
        context->skip--;
        return;
    }

    switch (context->selectorIndex) {
        case FULFILL_BASIC_ORDER:
            handle_fulfill_basic_order(msg, context);
            break;
        default:
            PRINTF("Selector Index not supported: %d\n", context->selectorIndex);
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
}
