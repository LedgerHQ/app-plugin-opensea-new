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
typedef enum {
    FULFILL_ORDER,
    FULFILL_BASIC_ORDER,
    FULFILL_AVAILABLE_ORDERS,
    FULFILL_ADVANCED_ORDER,
    FULFILL_AVAILABLE_ADVANCED_ORDERS,
    MATCH_ORDERS,
} selector_t;

// Number of selectors defined in this plugin. Should match the enum
// `selector_t`.
#define NUM_SELECTORS 6

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

typedef enum {
    MAO_OFFSET,
    MAO_CRITERIA_RESOLVERS_OFFSET,
    MAO_FULFILLMENTS_OFFSET,
    MAO_ADVANCED_ORDERS_LEN,
    MAO_ADVANCED_ORDERS,
    MAO_CRITERIA_AND_FULFILLMENTS,
} match_advanced_orders;

typedef enum {
    MO_OFFSET,
    MO_FULFILLMENT_OFFSET,
    MO_ORDERS_LEN,
    MO_ORDERS,
    MO_FULFILLMENT,
} match_orders;

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
    OFFER_ITEM_TYPE_NONE,
    OFFER_ITEM_TYPE_NATIVE,
    OFFER_ITEM_TYPE_ERC20,
    OFFER_ITEM_TYPE_NFT,
    OFFER_ITEM_TYPE_MULTIPLE_NFTS,
    OFFER_ITEM_TYPE_MULTIPLE_ERC20S,
    OFFER_ITEM_TYPE_MIXED_TYPES,
} offer_item_type;

typedef enum {
    OFFER_ITEM_TYPE,
    OFFER_TOKEN,
    OFFER_IDENTIFIER,
    OFFER_START_AMOUNT,
    OFFER_END_AMOUNT,
} offers;  // pointed to by context->items_index

typedef enum {
    CONSIDERATION_ITEM_TYPE_NONE,
    CONSIDERATION_ITEM_TYPE_NATIVE,
    CONSIDERATION_ITEM_TYPE_ERC20,
    CONSIDERATION_ITEM_TYPE_NFT,
    CONSIDERATION_ITEM_TYPE_MULTIPLE_NFTS,
    CONSIDERATION_ITEM_TYPE_MULTIPLE_ERC20S,
    CONSIDERATION_ITEM_TYPE_MIXED_TYPES,
} consideration_item_type;

typedef enum {
    CONSIDERATION_ITEM_TYPE,
    CONSIDERATION_TOKEN,
    CONSIDERATION_IDENTIFIER,
    CONSIDERATION_START_AMOUNT,
    CONSIDERATION_END_AMOUNT,
    CONSIDERATION_RECIPIENT,
} considerations;  // pointed to by context->items_index

typedef enum {
    TX_TYPE_NONE,
    TX_TYPE_ETH_NFT,
    TX_TYPE_ERC20_NFT,
    TX_TYPE_NFT_ERC20,
    TX_TYPE_NFT_NFT,
    TX_TYPE_MIX_NFT,
    TX_TYPE_NFT_MIX,
    TX_TYPE_TRANSFER_FROM,
    TX_TYPE_COULD_NOT_PARSE,
} tx_type;

typedef enum {
    BUY_NOW,
    ACCEPT_OFFER,
} sale_side;

/* structs */

//{
//    S_NONE,
//    S_BATCHED_INPUT_ORDERS,
//    S_BATCHED_OUTPUT_ORDERS,
//    S_ORDER,
//} on_struct;

/* 721 Standard TransferFrom Function */

// typedef enum
//{
//  TRANSFER_FROM__FROM,
//  TRANSFER_FROM__TO,
//  TRANSFER_FROM__TOKEN_ID,
//} transfer_from_parameter;

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

    // Struct parsing
    uint8_t orders_len;
    uint8_t orders_index;
    uint8_t param_index;
    uint8_t items_index;

    // Tx info
    uint8_t order_type;  // the nature of the tx (ETH_NFT, NFT_ERC20...)
    uint8_t booleans;    // bitwise booleans
    uint8_t tx_type;
    uint8_t sale_side;
    uint8_t offer_item_type;
    uint8_t consideration_item_type;
    uint16_t number_of_nfts;

    // ERC1155 info
    uint8_t numerator[16];
    uint8_t denominator[16];

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

// TOTAL =

// Piece of code that will check that the above structure is not bigger than 5
// * 32. Do not remove this check.
_Static_assert(sizeof(context_t) <= 5 * 32, "Structure of parameters too big.");

void handle_provide_parameter(void *parameters);
void handle_query_contract_ui(void *parameters);
void handle_init_contract(void *parameters);
void handle_finalize(void *parameters);
void handle_provide_token(void *parameters);
void handle_query_contract_id(void *parameters);
