#pragma once

#include "debug.h"
#include "eth_internals.h"
#include "eth_plugin_interface.h"
#include <stdint.h>
#include <string.h>

// Number of decimals used when the token wasn't found in the Crypto Asset List.
#define DEFAULT_DECIMAL WEI_TO_ETHER
#define ETH_DECIMAL     WEI_TO_ETHER

// Network tickers
#define MATIC         "MATIC "
#define WMATIC        "WMATIC "
#define ETH           "ETH "
#define WETH          "WETH "
#define UNKNOWN_ERC20 "? "
#define UNKNOWN_NFT   "NFT"

// Utility addresses checking
#define NULL_ADDRESS                    "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
#define ADDRESS_IS_NETWORK_TOKEN(_addr) (!memcmp(_addr, NETWORK_TOKEN_ADDRESS, ADDRESS_LENGTH))
#define ADDRESS_IS_NULL_ADDRESS(_addr)  (!memcmp(_addr, NULL_ADDRESS, ADDRESS_LENGTH))
extern const uint8_t NETWORK_TOKEN_ADDRESS[ADDRESS_LENGTH];

// Enumeration of the different selectors possible.
// Should follow the exact same order as the array declared in main.c
typedef enum { FULFILL_BASIC_ORDER } selector_t;

// Number of selectors defined in this plugin. Should match the enum
// `selector_t`.
#define NUM_SELECTORS 1

extern const uint32_t SEAPORT_SELECTORS[NUM_SELECTORS];

// Solidity itemType
typedef enum {
    NATIVE,
    ERC20,
    ERC721,
    ERC1155,
    ERC721_WITH_CRITERIA,
    ERC1155_WITH_CRITERIA
} item_type;

// Solidity basic_order_type abstraction.
typedef enum { ETH_NFT, ERC20_NFT, NFT_ERC20 } basic_order_type;

typedef enum {
    FBO__OFFSET_BASIC_ORDER_PARAM,
    FBO__CONSIDERATION_TOKEN,
    FBO__CONSIDERATION_IDENTIFIER,
    FBO__CONSIDERATION_AMOUNT,
    FBO__OFFERER,
    FBO__OFFER_TOKEN,
    FBO__OFFER_IDENTIFIER,
    FBO__OFFER_AMOUNT,
    FBO__BASIC_ORDER_TYPE,
    // FBO__TOTAL_ORIGINAL_ADDITIONAL_RECIPIENTS,
    FBO__LEN_ADDITIONAL_RECIPIENTS,
    FBO__ADDITIONAL_AMOUNT,
    // FBO__ADDITIONAL_RECIPIENT,
    FBO__LEN_SIGNATURE,
    // FBO__TMP
} fulfill_basic_order_parameter;

/* structs */

typedef enum {
    S_NONE,
    S_BATCHED_INPUT_ORDERS,
    S_BATCHED_OUTPUT_ORDERS,
    S_ORDER,
} on_struct;

// Booleans
#define BOOL1        (1)
#define BOOL2        (1 << 1)
#define BOOL3        (1 << 2)
#define ITEM1_IS_NFT (1 << 3)  // 0: ERC20/ETH, 1: NFT
#define ITEM2_IS_NFT (1 << 4)
#define ITEM1_FOUND  (1 << 5)
#define ITEM2_FOUND  (1 << 6)
#define IS_ETH       (1 << 7)

#define UPPER_P(x) x->elements[0]
#define LOWER_P(x) x->elements[1]
#define UPPER(x)   x.elements[0]
#define LOWER(x)   x.elements[1]

// screen array correspondance
#define SEND_UI        1  // Must remain first screen in screen array and always up.
#define SEND_UI_ERR    (1 << 1)
#define RECEIVE_UI     (1 << 2)
#define RECEIVE_UI_ERR (1 << 3)
#define LAST_UI        (1 << 7)  // Must remain last screen in array.

// Screens utility macros
#define RIGHT_SCROLL 1
#define LEFT_SCROLL  0

// Shared global memory with Ethereum app. Must be at most 5 * 32 bytes.
// 119 / 160
typedef struct __attribute__((__packed__)) context_t {
    // Parsing
    uint8_t next_param;
    uint16_t current_length;        // is the length of the current array
    uint32_t current_tuple_offset;  // is the value from which a given offset is calculated
    uint8_t skip;                   // number of parameters to skip

    // Tx info
    uint8_t order_type;  // the nature of the tx (ETH_NFT, NFT_ERC20...)
    uint8_t booleans;    // bitwise booleans

    // Token info
    uint8_t token1_address[ADDRESS_LENGTH];
    uint8_t token1_amount[INT256_LENGTH];
    uint8_t token2_address[ADDRESS_LENGTH];
    uint8_t token2_amount[INT256_LENGTH];

    // screen utils
    uint8_t screen_array;
    uint8_t previous_screen_index;
    uint8_t plugin_screen_index;

    // Method ID
    selector_t selectorIndex;
} context_t;

// Piece of code that will check that the above structure is not bigger than 5
// * 32. Do not remove this check.
_Static_assert(sizeof(context_t) <= 5 * 32, "Structure of parameters too big.");

void handle_provide_parameter(void *parameters);
void handle_query_contract_ui(void *parameters);
void handle_init_contract(void *parameters);
void handle_finalize(void *parameters);
void handle_provide_token(void *parameters);
void handle_query_contract_id(void *parameters);
