
#include "opensea_plugin.h"
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
    PRINTF("token.amount: %.*H\n", INT256_LENGTH, token.amount);
    PRINTF("token.address: %.*H\n", ADDRESS_LENGTH, token.address);
    context->transaction_info &IS_ACCEPT ? PRINTF("ACCEPT_OFFER\n") : PRINTF("BUY_NOW\n");
}
#endif

/*
**  Utils
*/

uint8_t get_basic_order_type(ethPluginProvideParameter_t *msg, uint8_t basic_order_type_sol) {
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
        default:
            return SOL_ERROR;
    }
}

/*
**  Parsing
*/

void parse_offer(ethPluginProvideParameter_t *msg, context_t *context) {
    PRINTF("PARSE OFFER\n");
    switch ((offers) context->items_index) {
        case OFFER_ITEM_TYPE:
            PRINTF("OFFER_ITEM_TYPE\n");
            PRINTF("OFFER_ITEM_TYPE CURRENT_LENGTH:%d\n", context->current_length);

            // only set token1.type on first Offer.
            if (context->token1.type == UNSET) {
                context->token1.type = get_item_type_from_sol(msg->parameter[PARAMETER_LENGTH - 1]);
                if (context->token1.type == SOL_ERROR) {
                    msg->result = ETH_PLUGIN_RESULT_ERROR;
                    return;
                }
                if (context->token1.type == ERC20 || context->token1.type == NATIVE)
                    context->transaction_info |= IS_ACCEPT;
            }
            // always set current_item_type
            context->current_item_type =
                get_item_type_from_sol(msg->parameter[PARAMETER_LENGTH - 1]);
            if (context->current_item_type == SOL_ERROR) {
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }

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
            if (!(context->transaction_info & IS_OFFER0_PARSED)) {
                // if start & end are different
                if (memcmp(context->token1.amount, msg->parameter, PARAMETER_LENGTH))
                    context->transaction_info |= IS_OFFER_DUTCH;
                context->transaction_info |= IS_OFFER0_PARSED;
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

void parse_considerations(ethPluginProvideParameter_t *msg, context_t *context) {
    PRINTF("PARSE CONSIDERATION\n");
    switch ((considerations) context->items_index) {
        case CONSIDERATION_ITEM_TYPE:
            PRINTF("CONSIDERATION_ITEM_TYPE\n");
            PRINTF("CONSIDERATION_ITEM_TYPE CURRENT_LENGTH:%d\n", context->current_length);

            // set t2.type only on consi[0]
            if (context->token2.type == UNSET)
                context->token2.type = get_item_type_from_sol(msg->parameter[PARAMETER_LENGTH - 1]);
            if (context->token2.type == SOL_ERROR) {
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }
            // always set current_item_type
            context->current_item_type =
                get_item_type_from_sol(msg->parameter[PARAMETER_LENGTH - 1]);
            if (context->current_item_type == SOL_ERROR) {
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }

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
            if (!(context->transaction_info & IS_CONSI0_PARSED)) {
                // if start & end are different
                if (memcmp(context->token2.amount, msg->parameter, PARAMETER_LENGTH))
                    context->transaction_info |= IS_CONSI_DUTCH;
                context->transaction_info |= IS_CONSI0_PARSED;
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

void parse_param(ethPluginProvideParameter_t *msg, context_t *context) {
    PRINTF("PARSE PARAM\n");
    switch ((parameters) context->param_index) {
        case PARAM_OFFERER:
            PRINTF("PARAM_OFFERER\n");
            // copy_address(context->offerer_address, msg->parameter, ADDRESS_LENGTH);
            context->skip = 9;
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
                PRINTF("OFFER_LEN ERROR\n");
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
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
            if (does_number_fit(msg->parameter,
                                PARAMETER_LENGTH,
                                sizeof(context->current_length))) {
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }
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

void parse_orders(ethPluginProvideParameter_t *msg, context_t *context) {
    switch ((orders) context->orders_index) {
        case ORDER_PARAMETER_OFFSET:
            PRINTF("ORDER_PARAMETER_OFFSET\n");

            if (does_number_fit(msg->parameter, PARAMETER_LENGTH, 1) ||
                msg->parameter[PARAMETER_LENGTH - 1] != 64) {
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }

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
            if (does_number_fit(msg->parameter,
                                PARAMETER_LENGTH,
                                sizeof(context->current_length))) {
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }
            context->current_length = U2BE(msg->parameter, PARAMETER_LENGTH - 2);

            // Skip signature, sometime length % 32 != 0
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

void parse_advanced_orders(ethPluginProvideParameter_t *msg, context_t *context) {
    switch ((advanced_orders) context->orders_index) {
        case ADVANCED_PARAMETER_OFFSET:
            PRINTF("ADVANCED_PARAMETER_OFFSET\n");

            if (does_number_fit(msg->parameter, PARAMETER_LENGTH, 1) ||
                msg->parameter[PARAMETER_LENGTH - 1] != 160) {
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }

            context->orders_index = ADVANCED_NUMERATOR;
            break;
        case ADVANCED_NUMERATOR:
            PRINTF("ADVANCED_NUMERATOR\n");

            if (does_number_fit(msg->parameter, PARAMETER_LENGTH, sizeof(context->numerator))) {
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
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            } else {
                context->denominator =
                    U4BE(msg->parameter, PARAMETER_LENGTH - sizeof(context->denominator));
                if (context->numerator && context->denominator &&
                    context->numerator != context->denominator)
                    context->transaction_info |= CANT_CALC_AMOUNT;
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
            if (does_number_fit(msg->parameter,
                                PARAMETER_LENGTH,
                                sizeof(context->current_length))) {
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }
            context->current_length = U2BE(msg->parameter, PARAMETER_LENGTH - 2);

            context->skip = context->current_length / PARAMETER_LENGTH;
            if (context->current_length % PARAMETER_LENGTH) context->skip++;

            context->orders_index = ADVANCED_EXTRADATA_LEN;
            break;
        case ADVANCED_EXTRADATA_LEN:
            PRINTF("ADVANCED_EXTRADATA_LEN\n");
            if (does_number_fit(msg->parameter,
                                PARAMETER_LENGTH,
                                sizeof(context->current_length))) {
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }
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
