#include "seaport_plugin.h"
#include "text.h"

/*
** Debug TMP
*/

static void debug_items(ethQueryContractUI_t *msg, context_t *context) {
    PRINTF("__debug_items()__\n");
    PRINTF("token1.address: %.*H\n", ADDRESS_LENGTH, context->token1.address);
    PRINTF("token2.address: %.*H\n", ADDRESS_LENGTH, context->token2.address);
    PRINTF("FLAG 10\n");
    PRINTF("token1.amount: %.*H\n", INT256_LENGTH, context->token1.amount);
    PRINTF("FLAG 11\n");
    PRINTF("token2.amount: %.*H\n", INT256_LENGTH, context->token2.amount);
    PRINTF("FLAG 12\n");

    if (msg->item1 != NULL)
        PRINTF("UI PENZO msg->item1->nft.collectionName: %s\n", msg->item1->nft.collectionName);
    else
        PRINTF("UI PENZO NO ITEM1\n");
    if (msg->item2 != NULL)
        PRINTF("UI PENZO msg->item2->nft.collectionName: %s\n", msg->item2->nft.collectionName);
    else
        PRINTF("UI PENZO NO ITEM2\n");
}

static void debug_tmp(__attribute__((unused)) ethQueryContractUI_t *msg, context_t *context) {
    PRINTF("ITEM1_FOUND: %d\n", context->booleans & ITEM1_FOUND ? 1 : 0);
    PRINTF("ITEM2_FOUND: %d\n", context->booleans & ITEM2_FOUND ? 1 : 0);
    PRINTF("ITEM1_IS_NFT: %d\n", context->booleans & ITEM1_IS_NFT ? 1 : 0);
    PRINTF("ITEM2_IS_NFT: %d\n", context->booleans & ITEM2_IS_NFT ? 1 : 0);
    PRINTF("IS_ETH: %d\n", context->booleans & IS_ETH ? 1 : 0);
}

/*
**  Items handler
*/

static void output_item1(ethQueryContractUI_t *msg, context_t *context) {
    PRINTF("____output_item1()____\n");
    debug_tmp(msg, context);
    if (context->booleans & ITEM1_FOUND) {     // item1 is found so is NOT ETH
        if (context->booleans & ITEM1_IS_NFT)  // item1 is found nft
            snprintf(msg->msg,
                     msg->msgLength,
                     "%d %s",
                     U4BE(context->token1.amount, INT256_LENGTH - 4),
                     msg->item1->nft.collectionName);  // TODO check U4BE() safety
        else                                           // item1 is found erc20
            amountToString(context->token1.amount,
                           INT256_LENGTH,
                           msg->item1->token.decimals,
                           msg->item1->token.ticker,
                           msg->msg,
                           msg->msgLength);
    } else {                                   // item1 not found
        if (context->booleans & ITEM1_IS_NFT)  // item1 is unknown nft
            snprintf(msg->msg,
                     msg->msgLength,
                     "%d %s",
                     U4BE(context->token1.amount, INT256_LENGTH - 4),
                     UNKNOWN_NFT);           // TODO check U4BE() safety
        else {                               // if item1 is money
            if (context->booleans & IS_ETH)  // if item1 is ETH
                amountToString(context->token1.amount,
                               INT256_LENGTH,
                               DEFAULT_DECIMAL,
                               ETH,
                               msg->msg,
                               msg->msgLength);
            else  // item1 is unknown erc20
                amountToString(context->token1.amount,
                               INT256_LENGTH,
                               DEFAULT_DECIMAL,
                               UNKNOWN_ERC20,
                               msg->msg,
                               msg->msgLength);
        }
    }
}

static void output_item2(ethQueryContractUI_t *msg, context_t *context) {
    PRINTF("____output_item2()____\n");
    debug_tmp(msg, context);
    if (!(context->booleans & ITEM1_FOUND) &&
        context->booleans & ITEM2_FOUND) {  // if item1 NOT found && item2 found
        if (context->booleans & ITEM2_IS_NFT)
            snprintf(msg->msg,
                     msg->msgLength,
                     "%d %s",
                     U4BE(context->token2.amount, INT256_LENGTH - 4),
                     msg->item1->nft.collectionName);  // TODO check U4BE() safety
        else {                                         // item2 is erc20
                // item1 is not a mistake, because if no item1, item2 become item1
            amountToString(context->token2.amount,
                           INT256_LENGTH,
                           msg->item1->token.decimals,
                           msg->item1->token.ticker,
                           msg->msg,
                           msg->msgLength);
        }

    } else if (context->booleans & ITEM1_FOUND &&
               context->booleans & ITEM2_FOUND) {  // if both item found (no eth)
        if (context->booleans & ITEM2_IS_NFT)
            snprintf(msg->msg,
                     msg->msgLength,
                     "%d %s",
                     U4BE(context->token2.amount, INT256_LENGTH - 4),
                     msg->item2->nft.collectionName);  // TODO check U4BE() safety
        else
            amountToString(context->token2.amount,
                           INT256_LENGTH,
                           msg->item2->token.decimals,
                           msg->item2->token.ticker,
                           msg->msg,
                           msg->msgLength);
    } else if (!(context->booleans & ITEM2_FOUND)) {  // if item2 not found

        if (!(context->booleans & ITEM2_IS_NFT)) {  // if item2 is money
            if (context->booleans & IS_ETH)
                amountToString(context->token2.amount,
                               INT256_LENGTH,
                               DEFAULT_DECIMAL,
                               ETH,
                               msg->msg,
                               msg->msgLength);
            else  // item2 is erc20
                amountToString(context->token2.amount,
                               INT256_LENGTH,
                               DEFAULT_DECIMAL,
                               UNKNOWN_ERC20,
                               msg->msg,
                               msg->msgLength);
        } else {  // item2 is nft
            snprintf(msg->msg,
                     msg->msgLength,
                     "%d %s",
                     U4BE(context->token2.amount, INT256_LENGTH - 4),
                     UNKNOWN_NFT);  // TODO check U4BE() safety
        }
    } else
        PRINTF("PENZO ERROR output_item2() did not handle that case.\n");
}

static void display_item(ethQueryContractUI_t *msg, token_t token, uint8_t is_found) {
    PRINTF("__display_item__\n");
    PRINTF("\ttoken.type: %d\n", token.type);
    switch (token.type) {
        case NATIVE:
            PRINTF("FLAG 1\n");
            amountToString(token.amount,
                           INT256_LENGTH,
                           DEFAULT_DECIMAL,
                           ETH,
                           msg->msg,
                           msg->msgLength);
            break;
        case ERC20:
            PRINTF("FLAG 2\n");
            PRINTF("current token found: %d\n", is_found);
            if (is_found)
                amountToString(token.amount,
                               INT256_LENGTH,
                               msg->item1->token.decimals,
                               msg->item1->token.ticker,
                               msg->msg,
                               msg->msgLength);
            // TODO handle ERC20 not found
            else
                amountToString(token.amount,
                               INT256_LENGTH,
                               DEFAULT_DECIMAL,
                               UNKNOWN_ERC20,
                               msg->msg,
                               msg->msgLength);
            break;
        case NFT:
            PRINTF("FLAG 3\n");
            // TODO check U4BE() safety
            PRINTF("case NFT, is_found: %d\n", is_found);
            if (is_found)
                snprintf(msg->msg,
                         msg->msgLength,
                         "%d %s",
                         U4BE(token.amount, INT256_LENGTH - 4),
                         msg->item2->nft.collectionName);  // TODO check U4BE() safety
            // msg->item1->nft.collectionName);  // TODO check U4BE() safety
            else
                snprintf(msg->msg,
                         msg->msgLength,
                         "%d %s",
                         U4BE(token.amount, INT256_LENGTH - 4),
                         UNKNOWN_NFT);
            // TODO handle found NFT
            break;
        case MULTIPLE_ERC20:
            PRINTF("FLAG 4\n");
            break;
        case MULTIPLE_NFTS:
            PRINTF("FLAG 5\n");
            break;
        default:
            PRINTF("ERROR UI display_item ERROR!!!!\n");
    }
}

/*
** Screens
*/

static void set_send_ui(ethQueryContractUI_t *msg, context_t *context) {
    // output_item1(msg, context);
    display_item(msg, context->token1, context->booleans & ITEM1_FOUND);
}
static void set_receive_ui(ethQueryContractUI_t *msg, context_t *context) {
    // output_item2(msg, context);
    display_item(msg, context->token2, context->booleans & ITEM2_FOUND);
}

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

/*
** Screens Utils
*/

static void skip_right(context_t *context) {
    while (!(context->screen_array & context->plugin_screen_index << 1))
        context->plugin_screen_index <<= 1;
    context->plugin_screen_index <<= 1;
}

static void skip_left(context_t *context) {
    while (!(context->screen_array & context->plugin_screen_index >> 1))
        context->plugin_screen_index >>= 1;
    context->plugin_screen_index >>= 1;
}

static bool get_scroll_direction(uint8_t screen_index, uint8_t previous_screen_index) {
    if (screen_index > previous_screen_index || screen_index == 0)
        return RIGHT_SCROLL;
    else
        return LEFT_SCROLL;
}

static void get_screen_array(ethQueryContractUI_t *msg, context_t *context) {
    if (msg->screenIndex == 0) {
        context->plugin_screen_index = SEND_UI;
        context->previous_screen_index = 0;
        return;
    }
    // This should only happen on last valid Screen
    if (msg->screenIndex == context->previous_screen_index) {
        context->plugin_screen_index = LAST_UI;
        // if LAST_UI is up, stop on it.
        if (context->screen_array & LAST_UI) return;
    }
    bool scroll_direction = get_scroll_direction(msg->screenIndex, context->previous_screen_index);
    // Save previous_screen_index after all checks are done.
    context->previous_screen_index = msg->screenIndex;
    // Scroll to next screen
    if (scroll_direction == RIGHT_SCROLL)
        skip_right(context);
    else
        skip_left(context);
}

/*
**  handle_query_contract_ui()
*/

void handle_query_contract_ui(void *parameters) {
    ethQueryContractUI_t *msg = (ethQueryContractUI_t *) parameters;
    context_t *context = (context_t *) msg->pluginContext;

    // Clean the display fields.
    memset(msg->title, 0, msg->titleLength);
    memset(msg->msg, 0, msg->msgLength);

    msg->result = ETH_PLUGIN_RESULT_OK;
    get_screen_array(msg, context);

    debug_items(msg, context);

    switch (context->plugin_screen_index) {
        case SEND_UI:
            strlcpy(msg->title, "Send", msg->titleLength);
            set_send_ui(msg, context);
            break;
        case SEND_UI_ERR:
            strlcpy(msg->title, "with address:", msg->titleLength);
            set_send_ui_err(msg, context);
            break;
        case RECEIVE_UI:
            strlcpy(msg->title, "Receive", msg->titleLength);
            set_receive_ui(msg, context);
            break;
        case RECEIVE_UI_ERR:
            strlcpy(msg->title, "with address:", msg->titleLength);
            set_receive_ui_err(msg, context);
            break;
            // case LAST_UI:
    }
}
