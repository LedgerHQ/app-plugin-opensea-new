#include "text.h"

void msg_display_address_ui(ethQueryContractUI_t *msg, uint8_t *address) {
  msg->msg[0] = '0';
  msg->msg[1] = 'x';
  getEthAddressStringFromBinary((uint8_t *)address, (char *)msg->msg + 2,
                                msg->pluginSharedRW->sha3, 0);
}

/**
 * @param token choose token1 or token2
 */
void msg_ticker_or_address(ethQueryContractUI_t *msg, context_t *context,
                           int token) {
  if (token == 1) // Token1
  {
    msg_display_address_ui(msg, context->token1_address);
  } else if (token == 2) // Token2
  {
    msg_display_address_ui(msg, context->token2_address);
  } else
    snprintf(msg->msg, msg->msgLength, "ERROR");
}

void msg_2tickers_ui(ethQueryContractUI_t *msg, context_t *context) {
  snprintf(msg->msg, msg->msgLength, "%s and %s", context->token1_ticker,
           context->token2_ticker);
}

/**
 * @param token choose token1 or token2
 */
void msg_number_of_tokens(ethQueryContractUI_t *msg, context_t *context,
                          int token) {
  if (context->number_of_tokens == 1) {
    snprintf(msg->msg, msg->msgLength, "%d token", context->number_of_tokens);
  } else
    snprintf(msg->msg, msg->msgLength, "%d tokens", context->number_of_tokens);
}

void msg_amount_or_address_ui(ethQueryContractUI_t *msg,
                              context_t *context) // Use for token1 only.
{
  msg_display_address_ui(msg, context->token1_address);
}
