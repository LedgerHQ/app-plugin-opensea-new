#include "seaport_plugin.h"

/*
**  Debug
*/

#ifdef DBG_PLUGIN
static void print_item(context_t *context, token_t token) {
    PRINTF("token.type: ");
    if (token.type == UNSET) PRINTF("UNSET\n");
    if (token.type == NATIVE) PRINTF("NATIVE\n");
    if (token.type == ERC20) PRINTF("ERC20\n");
    if (token.type == NFT) PRINTF("NFT\n");
    if (token.type == MULTIPLE_ERC20) PRINTF("MULTIPLE_ERC20\n");
    if (token.type == MULTIPLE_NFTS) PRINTF("MULTIPLE_NFTS\n");
    PRINTF("token.amount: %.*H", INT256_LENGTH, token.amount);
    PRINTF("token.address: %.*H", ADDRESS_LENGTH, token.address);
    context->booleans &IS_ACCEPT ? PRINTF("ACCEPT_OFFER\n") : PRINTF("BUY_NOW\n");
}
#endif

/*
**  Utils
*/

static uint8_t get_item_type_from_sol(uint8_t parameter_last_byte) {
    switch ((sol_ItemType_e) parameter_last_byte) {
        case SOL_NATIVE:
            return NATIVE;
        case SOL_ERC20:
            return ERC20;
        case SOL_ERC721:
        case SOL_ERC1155:
        case SOL_ERC721_WITH_CRITERIA:
        case SOL_ERC1155_WITH_CRITERIA:
            return NFT;
            break;
            // bad error handling
        default:
            return UNSET;
    }
}

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

static void parse_offer(ethPluginProvideParameter_t *msg, context_t *context) {
    PRINTF("PARSE OFFER\n");
    switch ((offers) context->items_index) {
        case OFFER_ITEM_TYPE:
            PRINTF("OFFER_ITEM_TYPE\n");
            PRINTF("OFFER_ITEM_TYPE CURRENT_LENGTH:%d\n", context->current_length);

            // only set token1.type on first Offer.
            if (context->token1.type == UNSET) {
                context->token1.type = get_item_type_from_sol(msg->parameter[PARAMETER_LENGTH - 1]);
                if (context->token1.type == ERC20)
                    context->booleans |= IS_ACCEPT;  // TODO check on NATIVE type too ?
            }
            // always set current_item_type
            context->current_item_type =
                get_item_type_from_sol(msg->parameter[PARAMETER_LENGTH - 1]);

#ifdef DBG_PLUGIN
            print_item(context, context->token1);  // utilitary
#endif
            context->items_index = OFFER_TOKEN;
            break;
        case OFFER_TOKEN:
            PRINTF("OFFER_TOKEN\n");

            if (context->token1.type == NATIVE) {
                if (context->current_item_type == ERC20) context->token1.type = MULTIPLE_ERC20;
            } else {  // token1.type != NATIVE
                // to set token1.address only on offer[0]
                if (ADDRESS_IS_NULL_ADDRESS(context->token1.address))
                    copy_address(context->token1.address, msg->parameter, ADDRESS_LENGTH);
                else {  // on offer[>0]
                    // is same type and different address as offer[0]
                    if (context->current_item_type == context->token1.type &&
                        memcmp(context->token1.address,
                               msg->parameter + PARAMETER_LENGTH - ADDRESS_LENGTH,
                               ADDRESS_LENGTH))
                        // change to multiple if a new address from the same type is found
                        context->token1.type =
                            (context->token1.type == ERC20) ? MULTIPLE_ERC20 : MULTIPLE_NFTS;
                }
            }

            context->items_index = OFFER_IDENTIFIER;
            break;
        case OFFER_IDENTIFIER:
            PRINTF("OFFER_IDENTIFIER\n");
            context->items_index = OFFER_START_AMOUNT;
            break;
        case OFFER_START_AMOUNT:
            PRINTF("OFFER_START_AMOUNT\n");

            // copy parameter in buffer
            uint8_t buf_amount[INT256_LENGTH] = {0};
            copy_parameter(buf_amount, msg->parameter, PARAMETER_LENGTH);
            PRINTF("BUF AMOUNT:\t%.*H\n", INT256_LENGTH, buf_amount);

            // add if t1.type is equal to current_item_type && t1.type is not multiple
            if (context->token1.type == context->current_item_type) {
                if (add_uint256(context->token1.amount, buf_amount)) {
                    PRINTF("ERROR: uint256 overflow error.\n");
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                }
            }
            // else add if MULTIPLE_NFTS
            else if (context->token1.type == MULTIPLE_NFTS && context->current_item_type == NFT) {
                if (add_uint256(context->token1.amount, buf_amount)) {
                    PRINTF("ERROR: uint256 overflow error.\n");
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                }
            }

            context->items_index = OFFER_END_AMOUNT;
            break;
        case OFFER_END_AMOUNT:
            PRINTF("OFFER_END_AMOUNT\n");

            // Only on order[0].offer[0]
            if (!(context->booleans & IS_OFFER0_PARSED)) {
                // if start & end are different
                if (memcmp(context->token1.amount, msg->parameter, PARAMETER_LENGTH))
                    context->booleans |= IS_OFFER_DUTCH;
                context->booleans |= IS_OFFER0_PARSED;
            }

            context->current_length--;
            context->items_index = OFFER_ITEM_TYPE;
            break;
        default:
            PRINTF("Param not supported: %d\n", context->next_param);
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
}

static void parse_considerations(ethPluginProvideParameter_t *msg, context_t *context) {
    PRINTF("PARSE CONSIDERATION\n");
    switch ((considerations) context->items_index) {
        case CONSIDERATION_ITEM_TYPE:
            PRINTF("CONSIDERATION_ITEM_TYPE\n");
            PRINTF("CONSIDERATION_ITEM_TYPE CURRENT_LENGTH:%d\n", context->current_length);

            // set t2.type only on consi[0]
            if (context->token2.type == UNSET)
                context->token2.type = get_item_type_from_sol(msg->parameter[PARAMETER_LENGTH - 1]);
            // always set current_item_type
            context->current_item_type =
                get_item_type_from_sol(msg->parameter[PARAMETER_LENGTH - 1]);

#ifdef DBG_PLUGIN
            print_item(context, context->token2);  // utilitary
#endif
            context->items_index = CONSIDERATION_TOKEN;
            break;
        case CONSIDERATION_TOKEN:
            PRINTF("CONSIDERATION_TOKEN\n");
            if (context->token2.type == NATIVE) {
                if (context->current_item_type == ERC20) context->token2.type = MULTIPLE_ERC20;
            } else {  // t2.type != NATIVE
                      // to set t2.address only on consi[0]
                if (ADDRESS_IS_NULL_ADDRESS(context->token2.address))
                    copy_address(context->token2.address, msg->parameter, ADDRESS_LENGTH);
                else {  // on consi[>0]
                    // is same type and different address as consi[0]
                    if (context->current_item_type == context->token2.type &&
                        memcmp(context->token2.address,
                               msg->parameter + PARAMETER_LENGTH - ADDRESS_LENGTH,
                               ADDRESS_LENGTH))
                        // change to multiple if a new address from the same type is found
                        context->token2.type =
                            (context->token2.type == ERC20) ? MULTIPLE_ERC20 : MULTIPLE_NFTS;
                }
            }
            context->items_index = CONSIDERATION_IDENTIFIER;
            break;
        case CONSIDERATION_IDENTIFIER:
            PRINTF("CONSIDERATION_IDENTIFIER\n");
            context->items_index = CONSIDERATION_START_AMOUNT;
            break;
        case CONSIDERATION_START_AMOUNT:
            PRINTF("CONSIDERATION_START_AMOUNT\n");

            uint8_t buf_amount[INT256_LENGTH] = {0};
            copy_parameter(buf_amount, msg->parameter, PARAMETER_LENGTH);
            PRINTF("BUF AMOUNT:\t%.*H\n", INT256_LENGTH, buf_amount);

            // add if t2.type is equal to current_item_type && t2.type is not multiple
            if (context->token2.type == context->current_item_type) {
                if (add_uint256(context->token2.amount, buf_amount)) {
                    PRINTF("ERROR: uint256 overflow error.\n");
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                }
            }
            // else add if MULTIPLE_NFTS
            else if (context->token2.type == MULTIPLE_NFTS && context->current_item_type == NFT) {
                if (add_uint256(context->token2.amount, buf_amount)) {
                    PRINTF("ERROR: uint256 overflow error.\n");
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                }
            }

            context->items_index = CONSIDERATION_END_AMOUNT;
            break;
        case CONSIDERATION_END_AMOUNT:
            PRINTF("CONSIDERATION_END_AMOUNT\n");

            // Only on order[0].consideration[0]
            if (!(context->booleans & IS_CONSI0_PARSED)) {
                // if start & end are different
                if (memcmp(context->token2.amount, msg->parameter, PARAMETER_LENGTH))
                    context->booleans |= IS_CONSI_DUTCH;
                context->booleans |= IS_CONSI0_PARSED;
            }

            context->items_index = CONSIDERATION_RECIPIENT;
            break;
        case CONSIDERATION_RECIPIENT:
            PRINTF("CONSIDERATION_RECIPIENT\n");
            context->current_length--;
            context->items_index = CONSIDERATION_ITEM_TYPE;
            break;
        default:
            PRINTF("Param not supported: %d\n", context->next_param);
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
}

/////////// NUMBER OF TOKENS + CURRENT SIZE VERIF ?

static void parse_param(ethPluginProvideParameter_t *msg, context_t *context) {
    PRINTF("PARSE PARAM\n");
    switch ((parameters) context->param_index) {
        case PARAM_OFFERER:
            PRINTF("PARAM_OFFERER\n");
            // copy_address(context->offerer_address, msg->parameter, ADDRESS_LENGTH);
            context->param_index = PARAM_ZONE;
            break;
        case PARAM_ZONE:
            PRINTF("PARAM_ZONE\n");
            context->param_index = PARAM_OFFER_OFFSET;
            break;
        case PARAM_OFFER_OFFSET:
            PRINTF("PARAM_OFFER_OFFSET\n");
            context->param_index = PARAM_CONSIDERATION_OFFSET;
            break;
        case PARAM_CONSIDERATION_OFFSET:
            PRINTF("PARAM_CONSIDERATION_OFFSET\n");
            context->param_index = PARAM_ORDER_TYPE;
            break;
        case PARAM_ORDER_TYPE:
            PRINTF("PARAM_ORDER_TYPE\n");
            context->param_index = PARAM_START_TIME;
            break;
        case PARAM_START_TIME:
            PRINTF("PARAM_START_TIME\n");
            context->param_index = PARAM_END_TIME;
            break;
        case PARAM_END_TIME:
            PRINTF("PARAM_END_TIME\n");
            context->param_index = PARAM_ZONE_HASH;
            break;
        case PARAM_ZONE_HASH:
            PRINTF("PARAM_ZONE_HASH\n");
            context->param_index = PARAM_SALT;
            break;
        case PARAM_SALT:
            PRINTF("PARAM_SALT\n");
            context->param_index = PARAM_CONDUIT_KEY;
            break;
        case PARAM_CONDUIT_KEY:
            PRINTF("PARAM_CONDUIT_KEY\n");
            context->param_index = PARAM_TOTAL_ORIGINAL_CONSIDERATION_ITEMS;
            break;
        case PARAM_TOTAL_ORIGINAL_CONSIDERATION_ITEMS:
            PRINTF("PARAM_TOTAL_ORIGINAL_CONSIDERATION_ITEMS\n");
            context->param_index = PARAM_OFFERS_LEN;
            context->items_index = 0;
            break;
        case PARAM_OFFERS_LEN:
            PRINTF("PARAM_OFFERS_LEN\n");
            context->current_length = U2BE(msg->parameter, PARAMETER_LENGTH - 2);
            if (context->current_length == 0) {
                PRINTF("NO OFFERS\n");
                context->param_index = PARAM_CONSIDERATIONS_LEN;
            } else
                context->param_index = PARAM_OFFERS;
            break;
        case PARAM_OFFERS:
            PRINTF("PARAM_OFFERS\n");
            PRINTF("OFFER CURRENT_LEN:%d\n", context->current_length);
            parse_offer(msg, context);
            if (context->current_length == 0) {
                context->param_index = PARAM_CONSIDERATIONS_LEN;
                context->items_index = 0;
            }
            break;
        case PARAM_CONSIDERATIONS_LEN:
            PRINTF("PARAM_CONSIDERATIONS_LEN\n");
            context->current_length = U2BE(msg->parameter, PARAMETER_LENGTH - 2);
            if (context->current_length == 0) {
                PRINTF("NO CONSIDERATIONS\n");
                context->token2.type = NATIVE;
                context->param_index = PARAM_END;
            } else
                context->param_index = PARAM_CONSIDERATIONS;
            break;
        case PARAM_CONSIDERATIONS:
            PRINTF("PARAM_CONSIDERATIONS\n");
            PRINTF("CONSIDERATION CURRENT_LEN:%d\n", context->current_length);
            parse_considerations(msg, context);
            if (context->current_length == 0) {
                context->param_index = PARAM_END;
                context->items_index = 0;
            }
            break;
        default:
            PRINTF("Param not supported: %d\n", context->param_index);
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
}

static void parse_orders(ethPluginProvideParameter_t *msg, context_t *context) {
    switch ((orders) context->orders_index) {
        case ORDER_PARAMETER_OFFSET:
            PRINTF("ORDER_PARAMETER_OFFSET\n");
            context->orders_index = ORDER_SIGNATURE_OFFSET;
            break;
        case ORDER_SIGNATURE_OFFSET:
            PRINTF("ORDER_SIGNATURE_OFFSET\n");
            context->orders_index = ORDER_PARAMETER;
            break;
        case ORDER_PARAMETER:
            PRINTF("ORDER_PARAMETER\n");
            parse_param(msg, context);
            if (context->param_index == PARAM_END) {
                PRINTF("PARAM END\n");
                context->param_index = 0;
                context->orders_index = ORDER_SIGNATURE;
            }
            break;
        case ORDER_SIGNATURE:
            PRINTF("ORDER_SIGNATURE\n");  // If len = 0 what happens ?
            context->current_length = U2BE(msg->parameter, PARAMETER_LENGTH - 2);

            context->skip = context->current_length / PARAMETER_LENGTH;
            if (context->current_length % PARAMETER_LENGTH) context->skip++;

            context->orders_len--;
            context->orders_index = ORDER_PARAMETER_OFFSET;
            break;
        default:
            PRINTF("Param not supported: %d\n", context->param_index);
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
}

static void parse_advanced_orders(ethPluginProvideParameter_t *msg, context_t *context) {
    switch ((advanced_orders) context->orders_index) {
        case ADVANCED_PARAMETER_OFFSET:
            PRINTF("ADVANCED_PARAMETER_OFFSET\n");

            if (does_number_fit(msg->parameter, PARAMETER_LENGTH, 1) ||
                msg->parameter[PARAMETER_LENGTH - 1] != 160) {
                context->booleans |= PARSE_ERROR;
                // TODO actually stop parsing when this happen.
            }

            context->orders_index = ADVANCED_NUMERATOR;
            break;
        case ADVANCED_NUMERATOR:
            PRINTF("ADVANCED_NUMERATOR\n");

            if (does_number_fit(msg->parameter, PARAMETER_LENGTH, sizeof(context->numerator))) {
                PRINTF("\n\n\nERROR, NUMBER DOES NOT FIT\n\n\n");
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            } else {
                context->numerator =
                    U4BE(msg->parameter, PARAMETER_LENGTH - sizeof(context->numerator));
            }

            context->orders_index = ADVANCED_DENOMINATOR;
            break;
        case ADVANCED_DENOMINATOR:
            PRINTF("ADVANCED_DENOMINATOR\n");

            if (does_number_fit(msg->parameter, PARAMETER_LENGTH, sizeof(context->denominator))) {
                PRINTF("\n\n\nERROR, NUMBER DOES NOT FIT\n\n\n");
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            } else {
                context->denominator =
                    U4BE(msg->parameter, PARAMETER_LENGTH - sizeof(context->denominator));
                if (context->numerator && context->denominator &&
                    context->numerator != context->denominator)
                    context->booleans |= CANT_CALC_AMOUNT;
            }

            context->orders_index = ADVANCED_SIGNATURE_OFFSET;
            break;
        case ADVANCED_SIGNATURE_OFFSET:
            PRINTF("ADVANCED_SIGNATURE_OFFSET\n");
            context->orders_index = ADVANCED_EXTRADATA_OFFSET;
            break;
        case ADVANCED_EXTRADATA_OFFSET:
            PRINTF("ADVANCED_EXTRADATA_OFFSET\n");
            context->orders_index = ADVANCED_PARAMETER;
            break;
        case ADVANCED_PARAMETER:
            PRINTF("ADVANCED_PARAMETER\n");
            parse_param(msg, context);
            if (context->param_index == PARAM_END) {
                PRINTF("PARAM END\n");
                context->param_index = 0;
                context->orders_index = ADVANCED_SIGNATURE_LEN;
            }
            break;
        case ADVANCED_SIGNATURE_LEN:
            PRINTF("ADVANCED_SIGNATURE_LEN\n");
            context->current_length = U2BE(msg->parameter, PARAMETER_LENGTH - 2);

            context->skip = context->current_length / PARAMETER_LENGTH;
            if (context->current_length % PARAMETER_LENGTH) context->skip++;

            context->orders_index = ADVANCED_EXTRADATA_LEN;
            break;
        case ADVANCED_EXTRADATA_LEN:
            PRINTF("ADVANCED_EXTRADATA_LEN\n");
            context->current_length = U2BE(msg->parameter, PARAMETER_LENGTH - 2);

            context->skip = context->current_length / PARAMETER_LENGTH;
            if (context->current_length % PARAMETER_LENGTH) context->skip++;

            context->orders_len--;
            context->orders_index = ADVANCED_PARAMETER_OFFSET;
            break;
        default:
            PRINTF("Param not supported: %d\n", context->param_index);
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
}

static void handle_cancel(ethPluginProvideParameter_t *msg, context_t *context) {
    switch ((match_advanced_orders) context->next_param) {
        case CANCEL_ORDER_OFFSET:
            PRINTF("CANCEL_ORDER_OFFSET\n");
            context->next_param = CANCEL_ORDERS_LEN;
            break;
        case CANCEL_ORDERS_LEN:
            PRINTF("CANCEL_ORDERS_LEN\n");
            if (U2BE(msg->parameter, PARAMETER_LENGTH - 2) > 1) {
                context->booleans |= ORDERS;
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

static void handle_match_advanced_orders(ethPluginProvideParameter_t *msg, context_t *context) {
    switch ((match_advanced_orders) context->next_param) {
        case MAO_OFFSET:
            PRINTF("MAO_OFFSET\n");
            context->next_param = MAO_CRITERIA_RESOLVERS_OFFSET;
            break;
        case MAO_CRITERIA_RESOLVERS_OFFSET:
            PRINTF("MAO_CRITERIA_RESOLVERS_OFFSET\n");
            context->next_param = MAO_FULFILLMENTS_OFFSET;
            break;
        case MAO_FULFILLMENTS_OFFSET:
            PRINTF("MAO_FULFILLMENTS_OFFSET\n");
            context->next_param = MAO_ADVANCED_ORDERS_LEN;
            break;
        case MAO_ADVANCED_ORDERS_LEN:
            PRINTF("MAO_ADVANCED_ORDERS_LEN\n");
            context->orders_len = U2BE(msg->parameter, PARAMETER_LENGTH - 2);
            context->skip = context->orders_len;
            PRINTF("ORDER_LEN FOUND:%d\n", context->orders_len);
            context->next_param = MAO_ADVANCED_ORDERS;
            break;
        case MAO_ADVANCED_ORDERS:
            PRINTF("MAO_ADVANCED_ORDERS\n");
            parse_advanced_orders(msg, context);
            PRINTF("PARSE ORDERS LEN:%d\n", context->orders_len);
            if (context->orders_len == 0) {
                PRINTF("END ORDERS\n");
                context->next_param = MAO_CRITERIA_AND_FULFILLMENTS;
                break;
            }
            break;
        case MAO_CRITERIA_AND_FULFILLMENTS:
            PRINTF("MAO_CRITERIA_AND_FULFILLMENTS\n");
            break;
        default:
            PRINTF("Param not supported: %d\n", context->param_index);
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            break;
    }
}

static void handle_match_orders(ethPluginProvideParameter_t *msg, context_t *context) {
    switch ((match_orders) context->next_param) {
        case MO_OFFSET:
            PRINTF("MO_OFFSET\n");
            context->next_param = MO_FULFILLMENT_OFFSET;
            break;
        case MO_FULFILLMENT_OFFSET:
            PRINTF("MO_FULFILLMENT_OFFSET\n");
            context->next_param = MO_ORDERS_LEN;
            break;
        case MO_ORDERS_LEN:
            PRINTF("MO_ORDERS_LEN\n");
            context->orders_len = U2BE(msg->parameter, PARAMETER_LENGTH - 2);
            context->skip = context->orders_len;
            PRINTF("ORDER_LEN FOUND:%d\n", context->orders_len);
            context->next_param = MO_ORDERS;
            break;
        case MO_ORDERS:
            PRINTF("MO_ORDERS\n");
            parse_orders(msg, context);
            PRINTF("PARSE ORDERS LEN:%d\n", context->orders_len);
            if (context->orders_len == 0) {
                PRINTF("END ORDERS\n");
                context->next_param = MO_FULFILLMENT;
                break;
            }
        case MO_FULFILLMENT:
            PRINTF("MO_FULFILLMENT\n");
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
            context->next_param = FAADO_CRITERIA_RESOLVERS_OFFSET;
            break;
        case FAADO_CRITERIA_RESOLVERS_OFFSET:
            PRINTF("FAADO_CRITERIA_RESOLVERS_OFFSET\n");
            context->next_param = FAADO_OFFER_FULFILLMENTS;
            break;
        case FAADO_OFFER_FULFILLMENTS:
            PRINTF("FAADO_OFFER_FULFILLMENTS\n");
            context->next_param = FAADO_CONSIDERATION_FULFILLMENTS;
            break;
        case FAADO_CONSIDERATION_FULFILLMENTS:
            PRINTF("FAADO_CONSIDERATION_FULFILLMENTS\n");
            context->next_param = FAADO_FULFILLER_CONDUIT_KEY;
            break;
        case FAADO_FULFILLER_CONDUIT_KEY:
            PRINTF("FAADO_FULFILLER_CONDUIT_KEY\n");
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
            context->orders_len = U2BE(msg->parameter, PARAMETER_LENGTH - 2);  // TODO: protect copy
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
                if (context->token2.type == NFT || context->token2.type == MULTIPLE_NFTS) {
                    // calc number of nfts using numerator and denominator
                    if (calc_number_of_nfts(context->token2.amount,
                                            context->numerator,
                                            context->denominator,
                                            &context->number_of_nfts)) {
                        msg->result =
                            ETH_PLUGIN_RESULT_ERROR;  // TODO check how to handle this error
                        break;
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
            context->next_param = FADO_CRITERIA_RESOLVERS_OFFSET;
            break;
        case FADO_CRITERIA_RESOLVERS_OFFSET:
            PRINTF("FADO_CRITERIA_RESOLVERS_OFFSET\n");
            context->next_param = FADO_FULFILLER_CONDUIT_KEY;
            break;
        case FADO_FULFILLER_CONDUIT_KEY:
            PRINTF("FADO_FULFILLER_CONDUIT_KEY\n");
            context->next_param = FADO_RECIPIENT;
            break;
        case FADO_RECIPIENT:
            PRINTF("FADO_RECIPIENT\n");
            copy_parameter(context->recipient_address, msg->parameter + 12, ADDRESS_LENGTH);
            context->next_param = FADO_PARAM_OFFSET;
            break;
        case FADO_PARAM_OFFSET:
            PRINTF("FADO_PARAM_OFFSET\n");
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
                    context->booleans |= CANT_CALC_AMOUNT;
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
                // TODO check if we should calc_number_of_nfts of token2 sometime
                if (context->token1.type == NFT || context->token1.type == MULTIPLE_NFTS) {
                    // calc number of nfts using numerator and denominator
                    if (calc_number_of_nfts(context->token1.amount,
                                            context->numerator,
                                            context->denominator,
                                            &context->number_of_nfts)) {
                        msg->result =
                            ETH_PLUGIN_RESULT_ERROR;  // TODO check how to handle this error
                        break;
                    }
                }
                if (context->token2.type == NFT || context->token2.type == MULTIPLE_NFTS) {
                    // calc number of nfts using numerator and denominator
                    if (calc_number_of_nfts(context->token2.amount,
                                            context->numerator,
                                            context->denominator,
                                            &context->number_of_nfts)) {
                        msg->result =
                            ETH_PLUGIN_RESULT_ERROR;  // TODO check how to handle this error
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
            context->next_param = FAO_OFFER_FULFILLMENT_OFFSET;
            break;
        case FAO_OFFER_FULFILLMENT_OFFSET:
            PRINTF("FAO_OFFER_FULFILLMENT_OFFSET\n");
            context->next_param = FAO_CONSIDERATION_FULFILLMENT_OFFSET;
            break;
        case FAO_CONSIDERATION_FULFILLMENT_OFFSET:
            PRINTF("FAO_CONSIDERATION_FULFILLMENT_OFFSET\n");
            context->next_param = FAO_FULFILLER_CONDUIT_KEY;
            break;
        case FAO_FULFILLER_CONDUIT_KEY:
            PRINTF("FAO_FULFILLER_CONDUIT_KEY\n");
            context->next_param = FAO_MAXIMUM_FULFILLED;
            break;
        case FAO_MAXIMUM_FULFILLED:
            PRINTF("FAO_MAXIMUM_FULFILLED\n");
            context->next_param = FAO_ORDERS_LEN;
            break;
        case FAO_ORDERS_LEN:
            PRINTF("FAO_ORDERS_LEN\n");
            context->orders_len = U2BE(msg->parameter, PARAMETER_LENGTH - 2);
            context->skip = context->orders_len;
            PRINTF("ORDER_LEN FOUND:%d\n", context->orders_len);
            context->next_param = FAO_ORDERS;
            break;
        case FAO_ORDERS:
            PRINTF("FAO_ORDERS\n");
            parse_orders(msg, context);
            PRINTF("PARSE ORDERS LEN:%d\n", context->orders_len);
            if (context->orders_len == 0) {
                PRINTF("END ORDERS\n");
                context->next_param = FAO_FULFILLMEMTS;
            }
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

static void handle_fullfill_order(ethPluginProvideParameter_t *msg, context_t *context) {
    switch ((fulfill_order) context->next_param) {
        case FO_OFFSET:
            PRINTF("FO_OFFSET\n");
            context->next_param = FO_FULFILLER_CONDUIT_KEY;
            break;
        case FO_FULFILLER_CONDUIT_KEY:
            PRINTF("FO_FULFILLER_CONDUIT_KEY\n");
            context->next_param = FO_ORDER_PARAM_OFFSET;
            break;
        case FO_ORDER_PARAM_OFFSET:
            PRINTF("FO_ORDER_PARAM_OFFSET\n");
            context->next_param = FO_ORDER_SIGNATURE_OFFSET;
            break;
        case FO_ORDER_SIGNATURE_OFFSET:
            PRINTF("FO_ORDER_SIGNATURE_OFFSET\n");
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
            uint8_t buf_amount[INT256_LENGTH] = {0};
            copy_parameter(buf_amount, msg->parameter, PARAMETER_LENGTH);
            PRINTF("BUF AMOUNT:\t%.*H\n", INT256_LENGTH, buf_amount);
            if (add_uint256(context->token1.amount, buf_amount)) {
                PRINTF("uint256 overflow error.\n");
                msg->result = ETH_PLUGIN_RESULT_ERROR;
            }
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
            handle_fullfill_order(msg, context);
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
        case MATCH_ORDERS:
            handle_match_orders(msg, context);
            break;
        case MATCH_ADVANCED_ORDERS:
            handle_match_advanced_orders(msg, context);
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
