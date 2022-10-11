#include "seaport_plugin.h"

static int find_selector(uint32_t selector, const uint32_t *selectors, size_t n,
                         selector_t *out)
{
  for (selector_t i = 0; i < n; i++)
  {
    if (selector == selectors[i])
    {
      *out = i;
      return 0;
    }
  }
  return -1;
}

// Called once to init.
void handle_init_contract(void *parameters)
{
  PRINTF("IN handle_init_contract\n");
  // Cast the msg to the type of structure we expect (here,
  // ethPluginInitContract_t).
  ethPluginInitContract_t *msg = (ethPluginInitContract_t *)parameters;

  // Make sure we are running a compatible version.
  if (msg->interfaceVersion != ETH_PLUGIN_INTERFACE_VERSION_LATEST)
  {
    // If not the case, return the `UNAVAILABLE` status.
    msg->result = ETH_PLUGIN_RESULT_UNAVAILABLE;
    return;
  }

  // Print size of plugin's context.
  PRINTF("context size: %d / %d\n", sizeof(context_t),
         msg->pluginContextLength);

  // Double check that the `context_t` struct is not bigger than the maximum
  // size (defined by `msg->pluginContextLength`).
  if (msg->pluginContextLength < sizeof(context_t))
  {
    PRINTF("Plugin parameters structure is bigger than allowed size\n");
    msg->result = ETH_PLUGIN_RESULT_ERROR;
    return;
  }

  context_t *context = (context_t *)msg->pluginContext;

  // Initialize the context (to 0).
  memset(context, 0, sizeof(*context));
  // Set current offset to the method id size
  context->current_tuple_offset = SELECTOR_SIZE;

  // Find tx selector
  uint32_t selector = U4BE(msg->selector, 0);
  if (find_selector(selector, SEAPORT_SELECTORS, NUM_SELECTORS,
                    &context->selectorIndex))
  {
    PRINTF("can't find selector\n");
    msg->result = ETH_PLUGIN_RESULT_UNAVAILABLE;
    return;
  }

  // Set `next_param` to be the first field we expect to parse.
  switch (context->selectorIndex)
  {
  case FULFILL_BASIC_ORDER:
    PRINTF("IN FULFILL_BASIC_ORDER\n");
    context->next_param = FBO__OFFSET_BASIC_ORDER_PARAM;
    break;
  case FULFILL_ORDER:
    PRINTF("IN FULFILL_ORDER");
    context->next_param = FO_ORDER_OFFSET;
    break;
  default:
    PRINTF("Missing selectorIndex: %d\n", context->selectorIndex);
    msg->result = ETH_PLUGIN_RESULT_ERROR;
    return;
  }

  // Return valid status.
  msg->result = ETH_PLUGIN_RESULT_OK;
}
