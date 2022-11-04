#include "opensea_plugin.h"

/*
**  Parsing
*/

static void handle_fulfill_basic_order(ethPluginProvideParameter_t *msg, context_t *context) {
    switch ((fulfill_basic_order_parameter) context->next_param) {
        case FBO__OFFSET_BASIC_ORDER_PARAM:
            PRINTF("FBO__OFFSET_BASIC_ORDER_PARAM\n");
            context->next_param = FBO__CONSIDERATION_TOKEN;
            break;
        case FBO__CONSIDERATION_TOKEN:
            PRINTF("FBO__CONSIDERATION_TOKEN\n");
            copy_address(context->token1.address, msg->parameter, ADDRESS_LENGTH);
            context->skip = 1;
            context->next_param = FBO__CONSIDERATION_AMOUNT;
            break;
        case FBO__CONSIDERATION_AMOUNT:
            PRINTF("FBO__CONSIDERATION_AMOUNT\n");
            copy_parameter(context->token1.amount, msg->parameter, PARAMETER_LENGTH);
            context->skip = 2;
            context->next_param = FBO__OFFER_TOKEN;
            break;
        case FBO__OFFER_TOKEN:
            PRINTF("FBO__OFFER_TOKEN\n");
            copy_address(context->token2.address, msg->parameter, PARAMETER_LENGTH);
            context->skip = 1;
            context->next_param = FBO__OFFER_AMOUNT;
            break;
        case FBO__OFFER_AMOUNT:
            PRINTF("FBO__OFFER_AMOUNT\n");
            copy_parameter(context->token2.amount, msg->parameter, PARAMETER_LENGTH);
            context->next_param = FBO__BASIC_ORDER_TYPE;
            break;
        case FBO__BASIC_ORDER_TYPE:
            PRINTF("FBO__BASIC_ORDER_TYPE\n");
            if (does_number_fit(msg->parameter, PARAMETER_LENGTH, 1)) {
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }
            context->order_type =
                get_basic_order_type(msg, U2BE(msg->parameter, PARAMETER_LENGTH - 2));
            context->skip = 9;
            context->next_param = FBO__LEN_ADDITIONAL_RECIPIENTS;
            break;
        case FBO__LEN_ADDITIONAL_RECIPIENTS:
            PRINTF("FBO__LEN_ADDITIONAL_RECIPIENTS\n");
            if (does_number_fit(msg->parameter,
                                PARAMETER_LENGTH,
                                sizeof(context->current_length))) {
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }
            context->current_length =
                U2BE(msg->parameter, PARAMETER_LENGTH - sizeof(context->current_length));
            context->next_param = FBO__ADDITIONAL_AMOUNT;
            break;
        case FBO__ADDITIONAL_AMOUNT:
            PRINTF("FBO__ADDITIONAL_AMOUNT\n");
            uint8_t buf_amount[INT256_LENGTH] = {0};
            if (context->order_type == ERC20_NFT || context->order_type == ETH_NFT) {
                copy_parameter(buf_amount, msg->parameter, PARAMETER_LENGTH);
                if (add_uint256(context->token1.amount, buf_amount)) {
                    PRINTF("ERROR: uint256 overflow error.\n");
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                }
            } else {  // is NFT_ERC20
                copy_parameter(buf_amount, msg->parameter, PARAMETER_LENGTH);
                if (sub_uint256(context->token2.amount, buf_amount)) {
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

static void handle_cancel(ethPluginProvideParameter_t *msg, context_t *context) {
    switch ((cancel) context->next_param) {
        case CANCEL_ORDER_OFFSET:
            PRINTF("CANCEL_ORDER_OFFSET\n");
            context->next_param = CANCEL_ORDERS_LEN;
            break;
        case CANCEL_ORDERS_LEN:
            PRINTF("CANCEL_ORDERS_LEN\n");
            // Check if there is multiple orders
            if (does_number_fit(msg->parameter, PARAMETER_LENGTH, 1) ||
                U2BE(msg->parameter, PARAMETER_LENGTH - 2) > 1) {
                context->transaction_info |= ORDERS;
            } else if (U2BE(msg->parameter, PARAMETER_LENGTH - 2) == 0) {
                PRINTF("ORDER_LEN ERROR\n");
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }
            context->next_param = CANCEL_ORDERS;
            break;
        case CANCEL_ORDERS:
            PRINTF("CANCEL_ORDERS\n");
            break;
        default:
            PRINTF("Param not supported: %d\n", context->param_index);
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
}

static void handle_fulfill_available_advanced_orders(ethPluginProvideParameter_t *msg,
                                                     context_t *context) {
    switch ((fulfill_available_advanced_orders) context->next_param) {
        case FAADO_OFFSET:
            PRINTF("FAADO_OFFSET\n");
            context->skip = 4;
            context->next_param = FAADO_RECIPIENT;
            break;
        case FAADO_RECIPIENT:
            PRINTF("FAADO_RECIPIENT\n");
            copy_parameter(context->recipient_address, msg->parameter + 12, ADDRESS_LENGTH);
            context->next_param = FAADO_MAXIMUM_FULFILLED;
            break;
        case FAADO_MAXIMUM_FULFILLED:
            PRINTF("FAADO_MAXIMUM_FULFILLED\n");
            context->next_param = FAADO_ORDERS_LEN;
            break;
        case FAADO_ORDERS_LEN:
            PRINTF("FAADO_ORDERS_LEN\n");
            if (does_number_fit(msg->parameter, PARAMETER_LENGTH, sizeof(context->orders_len))) {
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }
            context->orders_len = msg->parameter[PARAMETER_LENGTH - 1];
            if (context->orders_len == 0) {
                PRINTF("ORDER_LEN ERROR\n");
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }
            context->skip = context->orders_len;
            PRINTF("ORDER_LEN FOUND:%d\n", context->orders_len);
            context->next_param = FAADO_ORDERS;
            break;
        case FAADO_ORDERS:
            PRINTF("FAADO_ORDERS\n");
            parse_advanced_orders(msg, context);
            PRINTF("PARSE ORDERS LEN:%d\n", context->orders_len);
            if (context->orders_len == 0) {
                PRINTF("END ORDERS\n");
                // Calculate number_of_nfts at the end of the order.
                if (context->token2.type == NFT || context->token2.type == MULTIPLE_NFTS) {
                    // Calculate number of nfts using numerator and denominator.
                    if (calc_number_of_nfts(context->token2.amount,
                                            context->numerator,
                                            context->denominator,
                                            &context->number_of_nfts)) {
                        msg->result = ETH_PLUGIN_RESULT_ERROR;
                        return;
                    }
                }
                context->next_param = FAADO_CRITERIA_AND_FULFILLMENTS;
            }
            break;
        case FAADO_CRITERIA_AND_FULFILLMENTS:
            PRINTF("FAADO_CRITERIA_AND_FULFILLMENTS\n");
            break;
        default:
            PRINTF("Param not supported: %d\n", context->param_index);
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
}

static void handle_fulfill_advanced_order(ethPluginProvideParameter_t *msg, context_t *context) {
    switch ((fulfill_advanced_order) context->next_param) {
        case FADO_OFFSET:
            PRINTF("FADO_OFFSET\n");
            context->skip = 2;
            context->next_param = FADO_RECIPIENT;
            break;
        case FADO_RECIPIENT:
            PRINTF("FADO_RECIPIENT\n");
            copy_parameter(context->recipient_address, msg->parameter + 12, ADDRESS_LENGTH);
            context->skip = 1;
            context->next_param = FADO_NUMERATOR;
            break;
        case FADO_NUMERATOR:
            PRINTF("FADO_NUMERATOR\n");

            if (does_number_fit(msg->parameter, PARAMETER_LENGTH, sizeof(context->numerator))) {
                PRINTF("\n\n\nERROR, NUMBER DOES NOT FIT\n\n\n");
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            } else {
                context->numerator =
                    U4BE(msg->parameter, PARAMETER_LENGTH - sizeof(context->numerator));
            }

            context->next_param = FADO_DENOMINATOR;
            break;
        case FADO_DENOMINATOR:
            PRINTF("FADO_DENOMINATOR\n");

            if (does_number_fit(msg->parameter, PARAMETER_LENGTH, sizeof(context->denominator))) {
                PRINTF("\n\n\nERROR, NUMBER DOES NOT FIT\n\n\n");
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            } else {
                context->denominator =
                    U4BE(msg->parameter, PARAMETER_LENGTH - sizeof(context->denominator));
                if (context->numerator && context->denominator &&
                    context->numerator != context->denominator)
                    context->transaction_info |= CANT_CALC_AMOUNT;
            }

            context->next_param = FADO_SIGNATURE_OFFSET;
            break;
        case FADO_SIGNATURE_OFFSET:
            PRINTF("FADO_SIGNATURE_OFFSET\n");
            context->next_param = FADO_EXTRADATA_OFFSET;
            break;
        case FADO_EXTRADATA_OFFSET:
            PRINTF("FADO_EXTRADATA_OFFSET\n");
            context->next_param = FADO_PARAM;
            break;
        case FADO_PARAM:
            PRINTF("FADO_PARAM\n");
            parse_param(msg, context);
            if (context->param_index == PARAM_END) {
                PRINTF("PARAM END\n");
                context->param_index = 0;
                context->next_param = FADO_SIGNATURE;
                if (context->token1.type == NFT || context->token1.type == MULTIPLE_NFTS) {
                    // calc number of nfts using numerator and denominator
                    if (calc_number_of_nfts(context->token1.amount,
                                            context->numerator,
                                            context->denominator,
                                            &context->number_of_nfts)) {
                        msg->result = ETH_PLUGIN_RESULT_ERROR;
                        break;
                    }
                }
                if (context->token2.type == NFT || context->token2.type == MULTIPLE_NFTS) {
                    // calc number of nfts using numerator and denominator
                    if (calc_number_of_nfts(context->token2.amount,
                                            context->numerator,
                                            context->denominator,
                                            &context->number_of_nfts)) {
                        msg->result = ETH_PLUGIN_RESULT_ERROR;
                        break;
                    }
                }
            }
            break;
        case FADO_SIGNATURE:
            PRINTF("FADO_CRITERIA_RESOLVERS_OFFSET\n");
            break;
        default:
            PRINTF("Param not supported: %d\n", context->param_index);
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
}

static void handle_fulfill_available_orders(ethPluginProvideParameter_t *msg, context_t *context) {
    switch ((fulfill_available_orders) context->next_param) {
        case FAO_OFFSET:
            PRINTF("FAO_OFFSET\n");
            context->skip = 4;
            context->next_param = FAO_ORDERS_LEN;
            break;
        case FAO_ORDERS_LEN:
            PRINTF("FAO_ORDERS_LEN\n");
            if (does_number_fit(msg->parameter, PARAMETER_LENGTH, sizeof(context->orders_len))) {
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }
            context->orders_len = msg->parameter[PARAMETER_LENGTH - 1];
            if (context->orders_len == 0) {
                PRINTF("ORDERS_LEN ERROR\n");
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }
            context->skip = context->orders_len;
            PRINTF("ORDER_LEN FOUND:%d\n", context->orders_len);
            context->next_param = FAO_ORDERS;
            break;
        case FAO_ORDERS:
            PRINTF("FAO_ORDERS\n");
            parse_orders(msg, context);
            PRINTF("PARSE ORDERS LEN:%d\n", context->orders_len);
            if (context->orders_len == 0) context->next_param = FAO_FULFILLMEMTS;
            break;
        case FAO_FULFILLMEMTS:
            PRINTF("FAO_FULFILLMEMTS\n");
            break;
        default:
            PRINTF("Param not supported: %d\n", context->param_index);
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
}

static void handle_fulfill_order(ethPluginProvideParameter_t *msg, context_t *context) {
    switch ((fulfill_order) context->next_param) {
        case FO_OFFSET:
            PRINTF("FO_OFFSET\n");
            context->skip = 3;
            context->next_param = FO_ORDER_PARAM;
            break;
        case FO_ORDER_PARAM:
            PRINTF("FO_ORDER_PARAM\n");
            parse_param(msg, context);
            if (context->param_index == PARAM_END) {
                PRINTF("PARAM END\n");
                context->param_index = 0;
                context->next_param = FO_ORDER_SIGNATURE;
            }
            break;
        case FO_ORDER_SIGNATURE:
            PRINTF("FO_ORDER_SIGNATURE\n");
            break;
        default:
            PRINTF("Param not supported: %d\n", context->next_param);
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
}

static void handle_weth_withdraw(ethPluginProvideParameter_t *msg, context_t *context) {
    switch ((add_funds_eth) context->next_param) {
        case AMOUNT:
            PRINTF("ADD_FUNDS_AMOUNT\n$");
            copy_parameter(context->token1.amount, msg->parameter, PARAMETER_LENGTH);
            break;
        default:
            PRINTF("Param not supported: %d\n", context->next_param);
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
}

/*
**  handle_provide_parameter
*/

void handle_provide_parameter(void *parameters) {
    ethPluginProvideParameter_t *msg = (ethPluginProvideParameter_t *) parameters;
    context_t *context = (context_t *) msg->pluginContext;
    PRINTF(
        "___\nplugin provide parameter: offset %d\nBytes: \033[0;31m %.*H \n"
        "\033[0m \n",
        msg->parameterOffset,
        PARAMETER_LENGTH,
        msg->parameter);

    msg->result = ETH_PLUGIN_RESULT_OK;

    if (context->skip > 0) {
        PRINTF("SKIPPED parameter\n");
        context->skip--;
        return;
    }

    switch (context->selectorIndex) {
        case FULFILL_ORDER:
            handle_fulfill_order(msg, context);
            break;
        case FULFILL_BASIC_ORDER:
            handle_fulfill_basic_order(msg, context);
            break;
        case FULFILL_AVAILABLE_ORDERS:
            handle_fulfill_available_orders(msg, context);
            break;
        case FULFILL_ADVANCED_ORDER:
            handle_fulfill_advanced_order(msg, context);
            break;
        case FULFILL_AVAILABLE_ADVANCED_ORDERS:
            handle_fulfill_available_advanced_orders(msg, context);
            break;
        case CANCEL:
            handle_cancel(msg, context);
            break;
        case WETH_WITHDRAW:
            handle_weth_withdraw(msg, context);
            break;
        case INCREMENT_COUNTER:
        case WYVERN_V2_CANCEL_ORDER__:
        case WYVERN_V2_INCREMENT_NONCE:
        case WETH_DEPOSIT:
        case POLYGON_BRIDGE_DEPOSIT_ETHER_FOR:
        case ARBITRUM_BRIDGE_DEPOSIT_ETH:
        case OPTIMISM_BRIDGE_DEPOSIT_ETH:
        case UNISWAP_MULTICALL:
            break;
        default:
            PRINTF("Selector Index not supported: %d\n", context->selectorIndex);
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
}
