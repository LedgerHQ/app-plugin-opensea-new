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
#include "glyphs.h"

#include "opensea_plugin.h"

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
static const uint32_t UNISWAP_MULTICALL_SELECTOR = 0x5ae401dc;

// Array of all the different plugin selectors. Make sure this follows the same
// order as the enum defined in `opensea_plugin.h`
const uint32_t OPENSEA_SELECTORS[NUM_SELECTORS] = {
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
    UNISWAP_MULTICALL_SELECTOR,
};

const uint8_t STOREFRONT_LAZYMINTER[ADDRESS_LENGTH] = {0xa6, 0x04, 0x06, 0x08, 0x90, 0x92, 0x3f,
                                                       0xf4, 0x00, 0xe8, 0xc6, 0xf5, 0x29, 0x04,
                                                       0x61, 0xa8, 0x3a, 0xed, 0xac, 0xec};
