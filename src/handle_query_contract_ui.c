#include "seaport_plugin.h"
#include "text.h"

/*
** Debug TMP
*/

#ifdef DBG_PLUGIN
static void debug_items(ethQueryContractUI_t *msg, context_t *context) {
    PRINTF("\n__debug_items()__\n");
    PRINTF("\ttoken1.address: %.*H\n", ADDRESS_LENGTH, context->token1.address);
    PRINTF("\ttoken2.address: %.*H\n", ADDRESS_LENGTH, context->token2.address);
    PRINTF("\ttoken1.amount: %.*H\n", INT256_LENGTH, context->token1.amount);
    PRINTF("\ttoken2.amount: %.*H\n", INT256_LENGTH, context->token2.amount);
    PRINTF("\tnumber_of_nfts: %d\n", context->number_of_nfts);

    PRINTF("\tSell side: %s\n", (context->booleans & IS_ACCEPT) ? "Accept offer" : "Buy now");

    if (msg->item1 != NULL)
        PRINTF("UI PENZO msg->item1->nft.collectionName: %s\n", msg->item1->nft.collectionName);
    else
        PRINTF("UI PENZO NO ITEM1\n");
    if (msg->item2 != NULL)
        PRINTF("UI PENZO msg->item2->nft.collectionName: %s\n", msg->item2->nft.collectionName);
    else
        PRINTF("UI PENZO NO ITEM2\n");
}
static void debug_screens(ethQueryContractUI_t *msg, context_t *context) {
    PRINTF("SCREEN: screen_array:\t%d%d%d%d %d%d%d%d\n",
           context->screen_array & 1 ? 1 : 0,
           context->screen_array & (1 << 1) ? 1 : 0,
           context->screen_array & (1 << 2) ? 1 : 0,
           context->screen_array & (1 << 3) ? 1 : 0,
           context->screen_array & (1 << 4) ? 1 : 0,
           context->screen_array & (1 << 5) ? 1 : 0,
           context->screen_array & (1 << 6) ? 1 : 0,
           context->screen_array & (1 << 7) ? 1 : 0);
    PRINTF("SCREEN: screen_probe:\t%d%d%d%d %d%d%d%d\n",
           context->screen_probe & 1 ? 1 : 0,
           context->screen_probe & (1 << 1) ? 1 : 0,
           context->screen_probe & (1 << 2) ? 1 : 0,
           context->screen_probe & (1 << 3) ? 1 : 0,
           context->screen_probe & (1 << 4) ? 1 : 0,
           context->screen_probe & (1 << 5) ? 1 : 0,
           context->screen_probe & (1 << 6) ? 1 : 0,
           context->screen_probe & (1 << 7) ? 1 : 0);
    PRINTF("SCREEN: prev_screenIndex: %d\n", context->prev_screenIndex);
    PRINTF("SCREEN: msg->screenIndex: %d\n", msg->screenIndex);
}
#endif

/*
**  Items handler
*/

static void display_item(ethQueryContractUI_t *msg,
                         token_t token,
                         uint16_t number_of_nfts,
                         uint8_t is_found,
                         uint8_t no_amount) {
    PRINTF("__display_item__\n");
    PRINTF("\ttoken.type: %d\n", token.type);
    PRINTF("\ttoken.address: %.*H\n", ADDRESS_LENGTH, token.address);
    PRINTF("\ttoken.amount: %.*H\n", PARAMETER_LENGTH, token.amount);
    switch (token.type) {
        case NATIVE:
            // TODO CANC_CALC_AMOUNT
            if (no_amount) {
                snprintf(msg->msg, msg->msgLength, "? %s", ETH);
            } else {
                amountToString(token.amount,
                               INT256_LENGTH,
                               DEFAULT_DECIMAL,
                               ETH,
                               msg->msg,
                               msg->msgLength);
            }
            break;
        case ERC20:
            PRINTF("current token found: %d\n", is_found);
            if (no_amount) {
                snprintf(msg->msg, msg->msgLength, "? %s", msg->item1->token.ticker);
            } else {
                amountToString(token.amount,
                               INT256_LENGTH,
                               (is_found) ? msg->item1->token.decimals : DEFAULT_DECIMAL,
                               (is_found) ? msg->item1->token.ticker : UNKNOWN_ERC20,
                               msg->msg,
                               msg->msgLength);
            }
            break;
        case NFT:
        case MULTIPLE_NFTS:
            PRINTF("case NFT, is_found: %d\n", is_found);
            // TODO check U4BE() safety
            snprintf(msg->msg,
                     msg->msgLength,
                     "%d %s",
                     (number_of_nfts) ? number_of_nfts : U4BE(token.amount, INT256_LENGTH - 4),
                     (is_found) ? msg->item2->nft.collectionName : UNKNOWN_NFT);
            break;
        case MULTIPLE_ERC20:
            break;
        default:
            PRINTF("\n\n\nERROR UI display_item ERROR!!!!\n\n\n");
    }
}

/*
** Screens
*/

static void set_send_ui_err(ethQueryContractUI_t *msg, context_t *context) {
    msg->msg[0] = '0';
    msg->msg[1] = 'x';
    getEthAddressStringFromBinary((uint8_t *) context->token1.address,
                                  (char *) msg->msg + 2,
                                  msg->pluginSharedRW->sha3,
                                  0);
}

static void set_receive_ui_err(ethQueryContractUI_t *msg, context_t *context) {
    msg->msg[0] = '0';
    msg->msg[1] = 'x';
    getEthAddressStringFromBinary((uint8_t *) context->token2.address,
                                  (char *) msg->msg + 2,
                                  msg->pluginSharedRW->sha3,
                                  0);
}

static void set_add_funds_ui(ethQueryContractUI_t *msg, context_t *context) {
    // Set title
    if (context->selectorIndex == WETH_DEPOSIT)
        strlcpy(msg->title, "Wrap", msg->titleLength);
    else if (context->selectorIndex == POLYGON_BRIDGE_DEPOSIT_ETHER_FOR)
        strlcpy(msg->title, "To Polygon", msg->titleLength);
    else if (context->selectorIndex == ARBITRUM_BRIDGE_DEPOSIT_ETH)
        strlcpy(msg->title, "To Arbitrum", msg->titleLength);
    else if (context->selectorIndex == OPTIMISM_BRIDGE_DEPOSIT_ETH)
        strlcpy(msg->title, "To Optimism", msg->titleLength);
    else if (context->selectorIndex == WETH_WITHDRAW)
        strlcpy(msg->title, "Unwrap", msg->titleLength);
    else if (context->selectorIndex == UNISWAP_MULTICALL)
        strlcpy(msg->title, "Powered", msg->titleLength);
    else {
        msg->result = ETH_PLUGIN_RESULT_ERROR;
        return;
    }
    // Set msg
    if (context->selectorIndex == WETH_WITHDRAW) {
        amountToString(context->token1.amount,
                       INT256_LENGTH,
                       ETH_DECIMAL,
                       WETH,
                       msg->msg,
                       msg->msgLength);
    } else if (context->selectorIndex == UNISWAP_MULTICALL) {
        strlcpy(msg->msg, "by Uniswap Exchange", msg->msgLength);

    } else {
        amountToString(msg->pluginSharedRO->txContent->value.value,
                       msg->pluginSharedRO->txContent->value.length,
                       ETH_DECIMAL,
                       ETH,
                       msg->msg,
                       msg->msgLength);
    }
}

/*
**  handle_query_contract_ui()
*/

void handle_query_contract_ui(void *parameters) {
    ethQueryContractUI_t *msg = (ethQueryContractUI_t *) parameters;
    context_t *context = (context_t *) msg->pluginContext;

    msg->result = ETH_PLUGIN_RESULT_OK;

    // Clean the display fields.
    memset(msg->title, 0, msg->titleLength);
    memset(msg->msg, 0, msg->msgLength);

    // Return when no screens are set.
    if (context->screen_array == 0) {
        msg->result = ETH_PLUGIN_RESULT_ERROR;
        return;
    }

#ifdef DBG_PLUGIN
    debug_screens(msg, context);
#endif

    // Get current screen_probe
    context->screen_probe = get_screen_index(msg->screenIndex,
                                             &context->prev_screenIndex,
                                             context->screen_probe,
                                             context->screen_array);

#ifdef DBG_PLUGIN
    PRINTF("after get_screen_index()\n");
    debug_screens(msg, context);
    debug_items(msg, context);
#endif

    switch (context->screen_probe) {
        case PARSE_ERROR_UI:
            if (context->booleans & PARSE_ERROR) {
                strlcpy(msg->title, "Error:", msg->titleLength);
                strlcpy(msg->msg, "Could not parse transaction", msg->msgLength);
            } else if (context->booleans & CANT_CALC_AMOUNT) {
                strlcpy(msg->title, "Warning:", msg->titleLength);
                strlcpy(msg->msg, "Can't parse price.", msg->msgLength);
            } else {
                msg->result = ETH_PLUGIN_RESULT_ERROR;
                return;
            }
            break;
        case SEND_UI:
            strlcpy(msg->title,
                    (context->booleans & IS_CONSI_DUTCH) ? "Send max" : "Send",
                    msg->titleLength);
            display_item(msg,
                         context->token1,
                         context->number_of_nfts,
                         context->booleans & ITEM1_FOUND,
                         context->booleans & CANT_CALC_AMOUNT);
            break;
        case SEND_UI_ERR:
            if (context->token1.type == MULTIPLE_NFTS) {
                strlcpy(msg->title, "From", msg->titleLength);
                strlcpy(msg->msg, "multiple collections.", msg->msgLength);
            } else {
                strlcpy(msg->title, "with address:", msg->titleLength);
                set_send_ui_err(msg, context);
            }
            break;
        case RECEIVE_UI:
            if (context->booleans & IS_BUY4) {
                strlcpy(msg->title,
                        (context->booleans & IS_OFFER_DUTCH) ? "Transfer max" : "Transfer",
                        msg->titleLength);
            } else {
                strlcpy(msg->title,
                        (context->booleans & IS_OFFER_DUTCH) ? "Receive max" : "Receive",
                        msg->titleLength);
            }
            display_item(msg,
                         context->token2,
                         context->number_of_nfts,
                         context->booleans & ITEM2_FOUND,
                         context->booleans & CANT_CALC_AMOUNT);
            break;
        case RECEIVE_UI_ERR:
            if (context->token2.type == MULTIPLE_NFTS) {
                strlcpy(msg->title, "From", msg->titleLength);
                strlcpy(msg->msg, "multiple collections.", msg->msgLength);
            } else {
                strlcpy(msg->title, "with address:", msg->titleLength);
                set_receive_ui_err(msg, context);
            }
            break;
        case BUY_FOR_UI:
            strlcpy(msg->title, "Will be sent to", msg->titleLength);
            msg->msg[0] = '0';
            msg->msg[1] = 'x';
            getEthAddressStringFromBinary((uint8_t *) context->recipient_address,
                                          msg->msg + 2,
                                          msg->pluginSharedRW->sha3,
                                          0);
            break;
        case CANCEL_UI:
            strlcpy(msg->title, "Cancel", msg->titleLength);
            strlcpy(msg->msg, (context->booleans & ORDERS) ? "Orders" : "Order", msg->msgLength);
            break;
        case ADD_FUNDS_UI:
            set_add_funds_ui(msg, context);
            break;
        default:
            PRINTF("\n\n\n\n\n SCREEN NOT HANDLED\n\n\n\n\n");
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            return;
    }
}
