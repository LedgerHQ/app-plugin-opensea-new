#include "seaport_plugin.h"

static int find_selector(uint32_t selector, const uint32_t *selectors, size_t n, selector_t *out) {
    for (selector_t i = 0; i < n; i++) {
        if (selector == selectors[i]) {
            *out = i;
            return 0;
        }
    }
    return -1;
}

// Called once to init.
void handle_init_contract(void *parameters) {
    PRINTF("IN handle_init_contract\n");
    // Cast the msg to the type of structure we expect (here,
    // ethPluginInitContract_t).
    ethPluginInitContract_t *msg = (ethPluginInitContract_t *) parameters;

    // Make sure we are running a compatible version.
    if (msg->interfaceVersion != ETH_PLUGIN_INTERFACE_VERSION_LATEST) {
        // If not the case, return the `UNAVAILABLE` status.
        msg->result = ETH_PLUGIN_RESULT_UNAVAILABLE;
        return;
    }

    // Print size of plugin's context.
    PRINTF("context size: %d / %d\n", sizeof(context_t), msg->pluginContextLength);

    // Double check that the `context_t` struct is not bigger than the maximum
    // size (defined by `msg->pluginContextLength`).
    if (msg->pluginContextLength < sizeof(context_t)) {
        PRINTF("Plugin parameters structure is bigger than allowed size\n");
        msg->result = ETH_PLUGIN_RESULT_ERROR;
        return;
    }

    context_t *context = (context_t *) msg->pluginContext;

    // Initialize the context (to 0).
    memset(context, 0, sizeof(*context));
    // Set current offset to the method id size
    context->current_tuple_offset = SELECTOR_SIZE;

    // Find tx selector
    uint32_t selector = U4BE(msg->selector, 0);
    if (find_selector(selector, SEAPORT_SELECTORS, NUM_SELECTORS, &context->selectorIndex)) {
        PRINTF("can't find selector\n");
        msg->result = ETH_PLUGIN_RESULT_UNAVAILABLE;
        return;
    }

    // Set `next_param` to be the first field we expect to parse.
    switch (context->selectorIndex) {
        case FULFILL_ORDER:
            PRINTF("IN FULFILL_ORDER\n");
            context->next_param = FO_OFFSET;
            break;
        case FULFILL_BASIC_ORDER:
            PRINTF("IN FULFILL_BASIC_ORDER\n");
            context->next_param = FBO__OFFSET_BASIC_ORDER_PARAM;
            break;
        case FULFILL_AVAILABLE_ORDERS:
            PRINTF("IN FULFILL_AVAILABLE_ORDERS\n");
            context->next_param = FAO_OFFSET;
            break;
        case FULFILL_ADVANCED_ORDER:
            PRINTF("IN FULFILL_ADVANCED_ORDER\n");
            context->next_param = FADO_OFFSET;
            break;
        case FULFILL_AVAILABLE_ADVANCED_ORDERS:
            PRINTF("IN FULFILL_AVAILABLE_ADVANCED_ORDERS\n");
            context->next_param = FAADO_OFFSET;
            break;
        case MATCH_ORDERS:
            PRINTF("IN MATCH_ORDERS\n");
            context->next_param = MO_OFFSET;
            break;
        case MATCH_ADVANCED_ORDERS:
            PRINTF("IN MATCH_ADVANCED_ORDERS\n");
            context->next_param = MAO_OFFSET;
            break;
        case CANCEL:
            PRINTF("IN CANCEL\n");
            context->next_param = CANCEL_ORDER_OFFSET;
            break;
        case INCREMENT_COUNTER:
            PRINTF("IN INCREMENT_COUNTER\n");
            break;
        case WETH_DEPOSIT:
        case WETH_WITHDRAW:
        case POLYGON_BRIDGE_DEPOSIT_ETH:
        case ARBITRUM_BRIDGE_DEPOSIT_ETH:
        case OPTIMISM_BRIDGE_DEPOSIT_ETH:
            PRINTF("IN ADD_FUNDS\n");
            context->next_param = AMOUNT;
            break;
        default:
            PRINTF("Missing selectorIndex: %d\n", context->selectorIndex);
            msg->result = ETH_PLUGIN_RESULT_ERROR;
            return;
    }

    // Return valid status.
    msg->result = ETH_PLUGIN_RESULT_OK;
}
