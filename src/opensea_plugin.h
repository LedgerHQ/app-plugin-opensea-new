#pragma once

#include "debug.h"
#include "eth_plugin_interface.h"
#include "eth_internals.h"
#include <stdint.h>
#include <string.h>

// Number of decimals used when the token wasn't found in the Crypto Asset List.
#define DEFAULT_DECIMAL WEI_TO_ETHER
#define ETH_DECIMAL     WEI_TO_ETHER

#define PLUGIN_NAME "OpenSea"

// Network tickers
#define ETH           "ETH"
#define WETH          "WETH"
#define UNKNOWN_ERC20 "?"
#define UNKNOWN_NFT   "NFT"

// Utility addresses checking
#define NULL_ADDRESS                   "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
#define ADDRESS_IS_NULL_ADDRESS(_addr) (!memcmp(_addr, NULL_ADDRESS, ADDRESS_LENGTH))

#define OPENSEA_SHARED_STOREFRONT "OpenSea Shared Storefront"
extern const uint8_t STOREFRONT_LAZYMINTER[ADDRESS_LENGTH];

// Enumeration of the different selectors possible.
// Should follow the exact same order as the array declared in main.c
typedef enum {
    // Seaport
    FULFILL_ORDER,
    FULFILL_BASIC_ORDER,
    FULFILL_AVAILABLE_ORDERS,
    FULFILL_ADVANCED_ORDER,
    FULFILL_AVAILABLE_ADVANCED_ORDERS,
    CANCEL,
    INCREMENT_COUNTER,
    // Wyvern V2
    WYVERN_V2_CANCEL_ORDER__,
    WYVERN_V2_INCREMENT_NONCE,
    // Weth, Uniswap and bridge methods.
    WETH_DEPOSIT,
    WETH_WITHDRAW,
    POLYGON_BRIDGE_DEPOSIT_ETHER_FOR,
    ARBITRUM_BRIDGE_DEPOSIT_ETH,
    UNISWAP_MULTICALL,
} selector_t;

// Number of selectors defined in this plugin. Should match the enum
// `selector_t`.
#define NUM_SELECTORS 14

extern const uint32_t OPENSEA_SELECTORS[NUM_SELECTORS];

// Solidity basic_order_type abstraction.
typedef enum { ETH_NFT, ERC20_NFT, NFT_ERC20 } basic_order_type;

typedef enum {
    FO_OFFSET,
    FO_FULFILLER_CONDUIT_KEY,
    FO_ORDER_PARAM_OFFSET,
    FO_ORDER_SIGNATURE_OFFSET,
    FO_ORDER_PARAM,
    FO_ORDER_SIGNATURE,
} fulfill_order;

typedef enum {
    FBO__OFFSET_BASIC_ORDER_PARAM,
    FBO__CONSIDERATION_TOKEN,
    FBO__CONSIDERATION_AMOUNT,
    FBO__OFFER_TOKEN,
    FBO__OFFER_AMOUNT,
    FBO__BASIC_ORDER_TYPE,
    FBO__LEN_ADDITIONAL_RECIPIENTS,
    FBO__ADDITIONAL_AMOUNT,
    FBO__LEN_SIGNATURE,
} fulfill_basic_order_parameter;

typedef enum {
    FADO_OFFSET,
    FADO_CRITERIA_RESOLVERS_OFFSET,
    FADO_FULFILLER_CONDUIT_KEY,
    FADO_RECIPIENT,
    FADO_PARAM_OFFSET,
    FADO_NUMERATOR,
    FADO_DENOMINATOR,
    FADO_SIGNATURE_OFFSET,
    FADO_EXTRADATA_OFFSET,
    FADO_PARAM,
    FADO_SIGNATURE,
} fulfill_advanced_order;

typedef enum {
    FAADO_OFFSET,
    FAADO_CRITERIA_RESOLVERS_OFFSET,
    FAADO_OFFER_FULFILLMENTS,
    FAADO_CONSIDERATION_FULFILLMENTS,
    FAADO_FULFILLER_CONDUIT_KEY,
    FAADO_RECIPIENT,
    FAADO_MAXIMUM_FULFILLED,
    FAADO_ORDERS_LEN,
    FAADO_ORDERS,
    FAADO_CRITERIA_AND_FULFILLMENTS,
} fulfill_available_advanced_orders;

typedef enum {
    FAO_OFFSET,
    FAO_OFFER_FULFILLMENT_OFFSET,
    FAO_CONSIDERATION_FULFILLMENT_OFFSET,
    FAO_FULFILLER_CONDUIT_KEY,
    FAO_MAXIMUM_FULFILLED,
    FAO_ORDERS_LEN,
    FAO_ORDERS,
    FAO_FULFILLMEMTS,
} fulfill_available_orders;

typedef enum {
    CANCEL_ORDER_OFFSET,
    CANCEL_ORDERS_LEN,
    CANCEL_ORDERS,
} cancel;

typedef enum {
    AMOUNT,
} add_funds_eth;  // all methods for adding eth funds (bridge / wrap / unwrap eth) use same amount
// type.

typedef enum {
    ADVANCED_PARAMETER_OFFSET,
    ADVANCED_NUMERATOR,
    ADVANCED_DENOMINATOR,
    ADVANCED_SIGNATURE_OFFSET,
    ADVANCED_EXTRADATA_OFFSET,
    ADVANCED_PARAMETER,
    ADVANCED_SIGNATURE_LEN,
    ADVANCED_EXTRADATA_LEN,
} advanced_orders;

typedef enum {
    ORDER_PARAMETER_OFFSET,
    ORDER_SIGNATURE_OFFSET,
    ORDER_PARAMETER,
    ORDER_SIGNATURE,
} orders;

typedef enum {
    PARAM_OFFERER,
    PARAM_ZONE,
    PARAM_OFFER_OFFSET,
    PARAM_CONSIDERATION_OFFSET,
    PARAM_ORDER_TYPE,
    PARAM_START_TIME,
    PARAM_END_TIME,
    PARAM_ZONE_HASH,
    PARAM_SALT,
    PARAM_CONDUIT_KEY,
    PARAM_TOTAL_ORIGINAL_CONSIDERATION_ITEMS,
    PARAM_OFFERS_LEN,
    PARAM_OFFERS,
    PARAM_CONSIDERATIONS_LEN,
    PARAM_CONSIDERATIONS,
    PARAM_END,

} parameters;

typedef enum {
    ITEM_TYPE_NONE,
    ITEM_TYPE_NATIVE,
    ITEM_TYPE_ERC20,
    ITEM_TYPE_NFT,
    ITEM_TYPE_MULTIPLE_NFTS,
    ITEM_TYPE_MULTIPLE_ERC20S,
    ITEM_TYPE_MIXED_TYPES,
} item_type;

typedef enum {
    OFFER_ITEM_TYPE,
    OFFER_TOKEN,
    OFFER_IDENTIFIER,
    OFFER_START_AMOUNT,
    OFFER_END_AMOUNT,
} offers;  // pointed to by context->items_index

typedef enum {
    CONSIDERATION_ITEM_TYPE,
    CONSIDERATION_TOKEN,
    CONSIDERATION_IDENTIFIER,
    CONSIDERATION_START_AMOUNT,
    CONSIDERATION_END_AMOUNT,
    CONSIDERATION_RECIPIENT,
} considerations;  // pointed to by context->items_index

typedef enum sol_ItemType_e {
    SOL_NATIVE,
    SOL_ERC20,
    SOL_ERC721,
    SOL_ERC1155,
    SOL_ERC721_WITH_CRITERIA,
    SOL_ERC1155_WITH_CRITERIA,
    SOL_ERROR
} sol_ItemType_e;

typedef enum item_type_e {
    UNSET,
    NATIVE,
    ERC20,
    NFT,
    MULTIPLE_ERC20,
    MULTIPLE_NFTS,
} item_type_e;

// Booleans for transaction_info
#define IS_BUY4          (1)
#define IS_ACCEPT        (1 << 1)
#define ITEM1_IS_NFT     (1 << 2)
#define ITEM2_IS_NFT     (1 << 3)
#define ITEM1_FOUND      (1 << 4)
#define ITEM2_FOUND      (1 << 5)
#define CANT_CALC_AMOUNT (1 << 6)
#define ORDERS           (1 << 7)
#define IS_OFFER_DUTCH   (1 << 8)
#define IS_CONSI_DUTCH   (1 << 9)
#define IS_OFFER0_PARSED (1 << 10)
#define IS_CONSI0_PARSED (1 << 11)
#define IS_STOREFRONT    (1 << 12)

// screen array correspondance
#define PARSE_ERROR_UI 1
#define SEND_UI        (1 << 1)
#define SEND_UI_ERR    (1 << 2)
#define RECEIVE_UI     (1 << 3)
#define RECEIVE_UI_ERR (1 << 4)
#define BUY_FOR_UI     (1 << 5)
#define CANCEL_UI      (1 << 6)
#define ADD_FUNDS_UI   (1 << 7)

// screen boundaries
#define FIRST_UI 1
#define LAST_UI  (1 << 7)

// Screens utility macros
#define RIGHT_SCROLL 1
#define LEFT_SCROLL  0

// sizeof(token_t): 53
// aligned on 32
typedef struct token_t {
    uint8_t amount[INT256_LENGTH];
    uint8_t address[ADDRESS_LENGTH];
    uint8_t type;
} token_t;

// Shared global memory with Ethereum app. Must be at most 5 * 32 bytes.
// 160 / 160
typedef struct context_t {
    // aligned
    token_t token1;  // 53
    uint32_t denominator;
    uint32_t numerator;
    uint8_t orders_len;
    uint8_t orders_index;
    uint8_t param_index;
    // +64

    token_t token2;  // 53
    uint8_t items_index;
    uint8_t current_item_type;
    uint8_t order_type;  // the nature of the tx (ETH_NFT, NFT_ERC20...)
    uint8_t tx_type;
    uint16_t current_length;    // is the length of the current array
    uint16_t transaction_info;  // bitwise booleans
    uint16_t number_of_nfts;
    uint8_t next_param;
    // +64

    uint8_t recipient_address[ADDRESS_LENGTH];
    uint8_t skip;  // number of parameters to skip
    // screen utils
    uint8_t screen_array;
    uint8_t prev_screenIndex;
    uint8_t screen_probe;
    // Method ID
    selector_t selectorIndex;
    // +20 + 9(uint8) = 29
} context_t;

// TOTAL = 160

// Piece of code that will check that the above structure is not bigger than 5
// * 32. Do not remove this check.
_Static_assert(sizeof(context_t) <= 5 * 32, "Structure of parameters too big.");

uint8_t add_uint256(uint8_t *a, uint8_t *b);
uint8_t sub_uint256(uint8_t *a, uint8_t *b);
uint8_t does_number_fit(const uint8_t *parameter, uint8_t parameter_length, uint8_t size);
uint8_t calc_number_of_nfts(uint8_t *amount,
                            uint32_t numerator,
                            uint32_t denominator,
                            uint16_t *number_of_nfts);
void swap_tokens(context_t *context);
uint8_t get_screen_index(uint8_t screenIndex,
                         uint8_t *prevIndex,
                         uint8_t probe,
                         uint8_t screen_array);

void parse_offer(ethPluginProvideParameter_t *msg, context_t *context);
void parse_considerations(ethPluginProvideParameter_t *msg, context_t *context);
void parse_param(ethPluginProvideParameter_t *msg, context_t *context);
void parse_orders(ethPluginProvideParameter_t *msg, context_t *context);
void parse_advanced_orders(ethPluginProvideParameter_t *msg, context_t *context);
uint8_t get_basic_order_type(ethPluginProvideParameter_t *msg, uint8_t basic_order_type_sol);
