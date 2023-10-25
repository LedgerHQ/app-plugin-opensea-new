#include <stdbool.h>
#include "opensea_plugin.h"

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

    PRINTF("\tSell side: %s\n",
           (context->transaction_info & IS_ACCEPT) ? "Accept offer" : "Buy now");

    if (msg->item1 != NULL)
        PRINTF("UI msg->item1->nft.collectionName: %s\n", msg->item1->nft.collectionName);
    else
        PRINTF("UI NO ITEM1\n");
    if (msg->item2 != NULL) PRINTF("UI NO ITEM2\n");
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

static bool display_item(ethQueryContractUI_t *msg,
                         token_t token,
                         uint16_t number_of_nfts,
                         uint8_t is_found,
                         uint8_t no_amount,
                         uint8_t is_opensea_collection) {
    PRINTF("__display_item__\n");
    PRINTF("\ttoken.type: %d\n", token.type);
    PRINTF("\ttoken.address: %.*H\n", ADDRESS_LENGTH, token.address);
    PRINTF("\ttoken.amount: %.*H\n", PARAMETER_LENGTH, token.amount);
    switch (token.type) {
        case NATIVE:
            if (no_amount) {
                snprintf(msg->msg, msg->msgLength, "? %s", msg->network_ticker);
            } else {
                if (!amountToString(token.amount,
                                    INT256_LENGTH,
                                    DEFAULT_DECIMAL,
                                    msg->network_ticker,
                                    msg->msg,
                                    msg->msgLength)) {
                    return false;
                }
            }
            break;
        case ERC20:
            PRINTF("current token found: %d\n", is_found);
            if (no_amount) {
                snprintf(msg->msg, msg->msgLength, "? %s", msg->item1->token.ticker);
            } else {
                if (!amountToString(token.amount,
                                    INT256_LENGTH,
                                    (is_found) ? msg->item1->token.decimals : DEFAULT_DECIMAL,
                                    (is_found) ? msg->item1->token.ticker : UNKNOWN_ERC20,
                                    msg->msg,
                                    msg->msgLength)) {
                    return false;
                }
            }
            break;
        case NFT:
        case MULTIPLE_NFTS:
            PRINTF("case NFT, is_found: %d\n", is_found);
            if (does_number_fit(token.amount, INT256_LENGTH, 4)) {
                return false;
            }
            snprintf(msg->msg,
                     msg->msgLength,
                     "%d %s",
                     (number_of_nfts) ? number_of_nfts : U4BE(token.amount, INT256_LENGTH - 4),
                     (is_found) ? ((is_opensea_collection) ? OPENSEA_SHARED_STOREFRONT
                                                           : msg->item2->nft.collectionName)
                                : UNKNOWN_NFT);
            break;
        case MULTIPLE_ERC20:
            break;
        default:
            PRINTF("\n\n\nERROR UI display_item ERROR!!!!\n\n\n");
            return false;
    }
    return true;
}

/*
** Screens
*/

static bool set_send_ui_err(ethQueryContractUI_t *msg, context_t *context) {
    msg->msg[0] = '0';
    msg->msg[1] = 'x';
    return getEthAddressStringFromBinary((uint8_t *) context->token1.address,
                                         (char *) msg->msg + 2,
                                         msg->pluginSharedRW->sha3,
                                         0);
}

static bool set_receive_ui_err(ethQueryContractUI_t *msg, context_t *context) {
    msg->msg[0] = '0';
    msg->msg[1] = 'x';
    return getEthAddressStringFromBinary((uint8_t *) context->token2.address,
                                         (char *) msg->msg + 2,
                                         msg->pluginSharedRW->sha3,
                                         0);
}

static bool set_eth_add_funds_ui(ethQueryContractUI_t *msg, context_t *context) {
    // Set title
    if (context->selectorIndex == WETH_DEPOSIT)
        strlcpy(msg->title, "Wrap", msg->titleLength);
    else if (context->selectorIndex == POLYGON_BRIDGE_DEPOSIT_ETHER_FOR)
        strlcpy(msg->title, "To Polygon", msg->titleLength);
    else if (context->selectorIndex == ARBITRUM_BRIDGE_DEPOSIT_ETH)
        strlcpy(msg->title, "To Arbitrum", msg->titleLength);
    else if (context->selectorIndex == WETH_WITHDRAW)
        strlcpy(msg->title, "Unwrap", msg->titleLength);
    else if (context->selectorIndex == UNISWAP_MULTICALL)
        strlcpy(msg->title, "Powered", msg->titleLength);
    else {
        return false;
    }
    // Set msg
    if (context->selectorIndex == WETH_WITHDRAW) {
        if (!amountToString(context->token1.amount,
                            INT256_LENGTH,
                            DEFAULT_DECIMAL,
                            WETH,
                            msg->msg,
                            msg->msgLength)) {
            return false;
        }
    } else if (context->selectorIndex == UNISWAP_MULTICALL) {
        strlcpy(msg->msg, "by Uniswap Exchange", msg->msgLength);

    } else {
        if (!amountToString(msg->pluginSharedRO->txContent->value.value,
                            msg->pluginSharedRO->txContent->value.length,
                            ETH_DECIMAL,
                            ETH,
                            msg->msg,
                            msg->msgLength)) {
            return false;
        }
    }
    return true;
}

/*
**  handle_query_contract_ui()
*/

void handle_query_contract_ui(ethQueryContractUI_t *msg) {
    context_t *context = (context_t *) msg->pluginContext;
    bool ret = false;

    // Clean the display fields.
    memset(msg->title, 0, msg->titleLength);
    memset(msg->msg, 0, msg->msgLength);

    // Return when no screens are set.
    if (context->screen_array > 0) {
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
                if (context->transaction_info & CANT_CALC_AMOUNT) {
                    strlcpy(msg->title, "Warning:", msg->titleLength);
                    strlcpy(msg->msg, "Can't parse price.", msg->msgLength);
                    ret = true;
                }
                break;
            case SEND_UI:
                strlcpy(msg->title,
                        (context->transaction_info & IS_CONSI_DUTCH) ? "Send max" : "Send",
                        msg->titleLength);
                ret = display_item(msg,
                                   context->token1,
                                   context->number_of_nfts,
                                   context->transaction_info & ITEM1_FOUND,
                                   context->transaction_info & CANT_CALC_AMOUNT,
                                   0);
                break;
            case SEND_UI_ERR:
                if (context->token1.type == MULTIPLE_NFTS) {
                    strlcpy(msg->title, "From", msg->titleLength);
                    strlcpy(msg->msg, "multiple collections.", msg->msgLength);
                    ret = true;
                } else {
                    strlcpy(msg->title, "with address:", msg->titleLength);
                    ret = set_send_ui_err(msg, context);
                }
                break;
            case RECEIVE_UI:
                if (context->transaction_info & IS_BUY4) {
                    strlcpy(msg->title, "Transfer", msg->titleLength);
                } else {
                    strlcpy(msg->title, "Receive", msg->titleLength);
                }
                ret = display_item(msg,
                                   context->token2,
                                   context->number_of_nfts,
                                   context->transaction_info & ITEM2_FOUND,
                                   context->transaction_info & CANT_CALC_AMOUNT,
                                   (context->transaction_info & IS_STOREFRONT ? 1 : 0));
                break;
            case RECEIVE_UI_ERR:
                if (context->token2.type == MULTIPLE_NFTS) {
                    strlcpy(msg->title, "From", msg->titleLength);
                    strlcpy(msg->msg, "multiple collections.", msg->msgLength);
                    ret = true;
                } else {
                    strlcpy(msg->title, "with address:", msg->titleLength);
                    ret = set_receive_ui_err(msg, context);
                }
                break;
            case BUY_FOR_UI:
                strlcpy(msg->title, "Will be sent to", msg->titleLength);
                msg->msg[0] = '0';
                msg->msg[1] = 'x';
                ret = getEthAddressStringFromBinary((uint8_t *) context->recipient_address,
                                                    msg->msg + 2,
                                                    msg->pluginSharedRW->sha3,
                                                    0);
                break;
            case CANCEL_UI:
                strlcpy(msg->title, "Cancel", msg->titleLength);
                strlcpy(msg->msg,
                        (context->transaction_info & ORDERS) ? "Orders" : "Order",
                        msg->msgLength);
                ret = true;
                break;
            case ADD_FUNDS_UI:
                ret = set_eth_add_funds_ui(msg, context);
                break;
            default:
                PRINTF("SCREEN NOT HANDLED\n");
        }
    }
    msg->result = ret ? ETH_PLUGIN_RESULT_OK : ETH_PLUGIN_RESULT_ERROR;
}
