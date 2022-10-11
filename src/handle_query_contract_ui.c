#include "seaport_plugin.h"
#include "text.h"

static void handle_fulfill_basic_order_ui(ethQueryContractUI_t *msg,
                                          context_t *context) {
  switch (msg->screenIndex) {
  case 0:
    strlcpy(msg->title, "Send", msg->titleLength);
    switch ((basic_order_type)context->order_type) {
    case NFT_ERC20:
      snprintf(msg->msg, msg->msgLength, "%d %s", context->number_of_tokens,
               context->token1_ticker);
      break;
    case ETH_NFT:
      amountToString(context->token1_amount, INT256_LENGTH, WEI_TO_ETHER, ETH,
                     msg->msg, msg->msgLength);
      break;
    default:
      PRINTF("ORDER_TYPE NOT HANDLED\n");
      break;
    }
    break;
  case 1:
    strlcpy(msg->title, "Receive", msg->titleLength);
    if (context->order_type == NFT_ERC20) {
      amountToString(context->token2_amount, INT256_LENGTH,
                     context->token2_decimals, context->token2_ticker, msg->msg,
                     msg->msgLength);
    } else if (context->order_type == ETH_NFT) {
      snprintf(msg->msg, msg->msgLength, "%d %s", context->number_of_tokens,
               context->token2_ticker);
      // amountToString(context->token2_amount, INT256_LENGTH,
      //                context->token2_decimals, context->token2_ticker,
      //                msg->msg, msg->msgLength);
    }
    // msg_number_of_tokens(msg, context, 2);
    break;
  default:
    strlcpy(msg->title, "ERROR", msg->titleLength);
    strlcpy(msg->msg, "ERROR", msg->msgLength);
    break;
  }
}

void handle_query_contract_ui(void *parameters) {
  ethQueryContractUI_t *msg = (ethQueryContractUI_t *)parameters;
  context_t *context = (context_t *)msg->pluginContext;

  // Clean the display fields.
  memset(msg->title, 0, msg->titleLength);
  memset(msg->msg, 0, msg->msgLength);

  msg->result = ETH_PLUGIN_RESULT_OK;

  // Get according screen.
  switch (context->selectorIndex) {
  case FULFILL_BASIC_ORDER:
    handle_fulfill_basic_order_ui(msg, context);
    break;
  default:
    PRINTF("Error in handle_query_contract_ui's selectorIndex switch\n");
    msg->result = ETH_PLUGIN_RESULT_ERROR;
    break;
  }
}
