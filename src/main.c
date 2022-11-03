/*******************************************************************************
 *   Ethereum 2 Deposit Application
 *   (c) 2020 Ledger
 *
 *  Licensed under the Apache License, Version 2.0 (the "License");
 *  you may not use this file except in compliance with the License.
 *  You may obtain a copy of the License at
 *
 *      http://www.apache.org/licenses/LICENSE-2.0
 *
 *  Unless required by applicable law or agreed to in writing, software
 *  distributed under the License is distributed on an "AS IS" BASIS,
 *  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
 *  See the License for the specific language governing permissions and
 *  limitations under the License.
 ********************************************************************************/

#include <stdbool.h>
#include <stdint.h>
#include <string.h>

#include "cx.h"
#include "os.h"

#include "seaport_plugin.h"

// List of selectors supported by this plugin.
static const uint32_t FULFILL_ORDER_SELECTOR = 0xb3a34c4c;
static const uint32_t FULFILL_BASIC_ORDER_SELECTOR = 0xfb0f3ee1;
static const uint32_t FULFILL_AVAILABLE_ORDERS_SELECTOR = 0xed98a574;
static const uint32_t FULFILL_ADVANCED_ORDER_SELECTOR = 0xe7acab24;
static const uint32_t FULFILL_AVAILABLE_ADVANCED_ORDERS_SELECTOR = 0x87201b41;
static const uint32_t CANCEL_SELECTOR = 0xfd9f1e10;
static const uint32_t INCREMENT_COUNTER_SELECTOR = 0x5b34b966;
static const uint32_t WYVERN_V2_CANCEL_ORDER__SELECTOR = 0xa8a41c70;
static const uint32_t WYVERN_V2_INCREMENT_NONCE_SELECTOR = 0x627cdcb9;
static const uint32_t WETH_DEPOSIT_SELECTOR = 0xd0e30db0;
static const uint32_t WETH_WITHDRAW_SELECTOR = 0x2e1a7d4d;
static const uint32_t POLYGON_BRIDGE_DEPOSIT_ETHER_FOR_SELECTOR = 0x4faa8a26;
static const uint32_t ARBITRUM_BRIDGE_DEPOSIT_ETH_SELECTOR = 0x439370b1;
static const uint32_t OPTIMISM_BRIDGE_DEPOSIT_ETH_SELECTOR = 0xb1a1a882;
static const uint32_t UNISWAP_MULTICALL_SELECTOR = 0x5ae401dc;

// Array of all the different plugin selectors. Make sure this follows the same
// order as the enum defined in `seaport_plugin.h`
const uint32_t SEAPORT_SELECTORS[NUM_SELECTORS] = {
    // Seaport Methods
    FULFILL_ORDER_SELECTOR,
    FULFILL_BASIC_ORDER_SELECTOR,
    FULFILL_AVAILABLE_ORDERS_SELECTOR,
    FULFILL_ADVANCED_ORDER_SELECTOR,
    FULFILL_AVAILABLE_ADVANCED_ORDERS_SELECTOR,
    CANCEL_SELECTOR,
    INCREMENT_COUNTER_SELECTOR,
    // OpenSea Wyvern V2 methods
    WYVERN_V2_CANCEL_ORDER__SELECTOR,
    WYVERN_V2_INCREMENT_NONCE_SELECTOR,
    // Bridge and weth methods
    WETH_DEPOSIT_SELECTOR,
    WETH_WITHDRAW_SELECTOR,
    POLYGON_BRIDGE_DEPOSIT_ETHER_FOR_SELECTOR,
    ARBITRUM_BRIDGE_DEPOSIT_ETH_SELECTOR,
    OPTIMISM_BRIDGE_DEPOSIT_ETH_SELECTOR,
    UNISWAP_MULTICALL_SELECTOR,
    // Previous Opensea contracts remainging active methods.
};

// Function to dispatch calls from the ethereum app.
void dispatch_plugin_calls(int message, void *parameters) {
    switch (message) {
        case ETH_PLUGIN_INIT_CONTRACT:
            handle_init_contract(parameters);
            break;
        case ETH_PLUGIN_PROVIDE_PARAMETER:
            handle_provide_parameter(parameters);
            break;
        case ETH_PLUGIN_FINALIZE:
            handle_finalize(parameters);
            break;
        case ETH_PLUGIN_PROVIDE_INFO:
            handle_provide_token(parameters);
            break;
        case ETH_PLUGIN_QUERY_CONTRACT_ID:
            handle_query_contract_id(parameters);
            break;
        case ETH_PLUGIN_QUERY_CONTRACT_UI:
            handle_query_contract_ui(parameters);
            break;
        default:
            PRINTF("Unhandled message %d\n", message);
            break;
    }
}

void handle_query_ui_exception(unsigned int *args) {
    switch (args[0]) {
        case ETH_PLUGIN_QUERY_CONTRACT_UI:
            ((ethQueryContractUI_t *) args[1])->result = ETH_PLUGIN_RESULT_ERROR;
            break;
        default:
            break;
    }
}

// Calls the ethereum app.
void call_app_ethereum() {
    unsigned int libcall_params[3];
    libcall_params[0] = (unsigned int) "Ethereum";
    libcall_params[1] = 0x100;
    libcall_params[2] = RUN_APPLICATION;
    os_lib_call((unsigned int *) &libcall_params);
}

// Weird low-level black magic. No need to edit this.
__attribute__((section(".boot"))) int main(int arg0) {
    // Exit critical section
    __asm volatile("cpsie i");

    // Ensure exception will work as planned
    os_boot();

    // Try catch block. Please read the docs for more information on how to use
    // those!
    BEGIN_TRY {
        TRY {
            // Low-level black magic.
            check_api_level(CX_COMPAT_APILEVEL);

            // Check if we are called from the dashboard.
            if (!arg0) {
                // Called from dashboard, launch Ethereum app
                call_app_ethereum();
                return 0;
            } else {
                // Not called from dashboard: called from the ethereum app!
                const unsigned int *args = (const unsigned int *) arg0;

                // If `ETH_PLUGIN_CHECK_PRESENCE` is set, this means the caller is just
                // trying to know whether this app exists or not. We can skip
                // `dispatch_plugin_calls`.
                if (args[0] != ETH_PLUGIN_CHECK_PRESENCE) {
                    dispatch_plugin_calls(args[0], (void *) args[1]);
                }
            }
        }
        CATCH_OTHER(e) {
            switch (e) {
                // These exceptions are only generated on handle_query_contract_ui()
                case 0x6502:
                case EXCEPTION_OVERFLOW:
                    handle_query_ui_exception((unsigned int *) arg0);
                    break;
                default:
                    break;
            }
            PRINTF("Exception 0x%x caught\n", e);
        }
        FINALLY {
            // Call `os_lib_end`, go back to the ethereum app.
            os_lib_end();
        }
    }
    END_TRY;

    // Will not get reached.
    return 0;
}
