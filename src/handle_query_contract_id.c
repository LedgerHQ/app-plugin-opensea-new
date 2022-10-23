#include "seaport_plugin.h"
#include "text.h"

// Sets the first screen to display.
void handle_query_contract_id(void *parameters) {
  ethQueryContractID_t *msg = (ethQueryContractID_t *)parameters;
  const context_t *context = (const context_t *)msg->pluginContext;
  // msg->name will be the upper sentence displayed on the screen.
  // msg->version will be the lower sentence displayed on the screen.

  // For the first screen, display the plugin name.
  strlcpy(msg->name, PLUGIN_NAME, msg->nameLength);

  // Get selector according screen.
  switch (context->selectorIndex) {
  case FULFILL_BASIC_ORDER:
    PRINTF("PENZO (basic_order_type)context->order_type: %d\n",
           context->order_type);
    strlcpy(msg->version, "exchange", msg->versionLength);
    break;
  default:
    PRINTF("Selector index: %d not supported\n", context->selectorIndex);
    msg->result = ETH_PLUGIN_RESULT_ERROR;
    return;
  }
  msg->result = ETH_PLUGIN_RESULT_OK;
}
