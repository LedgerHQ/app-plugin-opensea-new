#pragma once

#include "debug.h"
#include "eth_internals.h"
#include "eth_plugin_interface.h"
#include <string.h>

// Number of decimals used when the token wasn't found in the Crypto Asset List.
#define DEFAULT_DECIMAL WEI_TO_ETHER
#define ETH_DECIMAL WEI_TO_ETHER

// Network tickers
#define MATIC "MATIC "
#define WMATIC "WMATIC "
#define ETH "ETH "
#define WETH "WETH "

// Utility addresses checking
#define NULL_ADDRESS "\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0\0"
#define ADDRESS_IS_NETWORK_TOKEN(_addr)                                        \
  (!memcmp(_addr, NETWORK_TOKEN_ADDRESS, ADDRESS_LENGTH))
#define ADDRESS_IS_NULL_ADDRESS(_addr)                                         \
  (!memcmp(_addr, NULL_ADDRESS, ADDRESS_LENGTH))
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
  ETH_TO_ERC721,
  // ETH_TO_ERC721_PARTIAL_OPEN,
  // ETH_TO_ERC721_FULL_RESTRICTED,
  // ETH_TO_ERC721_PARTIAL_RESTRICTED,
  ETH_TO_ERC1155,
  // ETH_TO_ERC1155_PARTIAL_OPEN,
  // ETH_TO_ERC1155_FULL_RESTRICTED,
  // ETH_TO_ERC1155_PARTIAL_RESTRICTED,
  ERC20_TO_ERC721,
  // ERC20_TO_ERC721_PARTIAL_OPEN,
  // ERC20_TO_ERC721_FULL_RESTRICTED,
  // ERC20_TO_ERC721_PARTIAL_RESTRICTED,
  ERC20_TO_ERC1155,
  // ERC20_TO_ERC1155_PARTIAL_OPEN,
  // ERC20_TO_ERC1155_FULL_RESTRICTED,
  // ERC20_TO_ERC1155_PARTIAL_RESTRICTED,
  ERC721_TO_ERC20,
  // ERC721_TO_ERC20_PARTIAL_OPEN,
  // ERC721_TO_ERC20_FULL_RESTRICTED,
  // ERC721_TO_ERC20_PARTIAL_RESTRICTED,
  ERC1155_TO_ERC20,
  // ERC1155_TO_ERC20_PARTIAL_OPEN,
  // ERC1155_TO_ERC20_FULL_RESTRICTED,
  // ERC1155_TO_ERC20_PARTIAL_RESTRICTED
} old_keep_for_ref;

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

/* 721 Standard TransferFrom Function */

typedef enum {
  TRANSFER_FROM__FROM,
  TRANSFER_FROM__TO,
  TRANSFER_FROM__TOKEN_ID,
} transfer_from_parameter;

// Booleans
#define BOOL1 (1)
#define BOOL2 (1 << 2)
#define BOOL3 (1 << 3)
#define BOOL4 (1 << 4)
#define BOOL5 (1 << 5)
#define BOOL6 (1 << 6)
#define BOOL7 (1 << 7)
#define BOOL8 (1 << 8)

#define UPPER_P(x) x->elements[0]
#define LOWER_P(x) x->elements[1]
#define UPPER(x) x.elements[0]
#define LOWER(x) x.elements[1]

typedef struct uint128_t {
  uint64_t elements[2];
} uint128_t;

typedef struct uint256_t {
  uint128_t elements[2];
} uint256_t;

// Shared global memory with Ethereum app. Must be at most 5 * 32 bytes.
// 124 / 160
typedef struct __attribute__((__packed__)) context_t {
  uint8_t on_struct;
  uint8_t next_param;
  // uint32_t next_offset;    // is the value of the next target offset
  uint16_t current_length; // is the length of the current array
  // uint16_t target_offset;  // is the offset of the parameter we want to parse
  uint32_t current_tuple_offset; // is the value from which a given offset is
                                 // calculated
  // uint32_t last_calldata_offset; // is the offset of the last order's
  // calldata end, just before the last byte of the Tx
  uint8_t number_of_tokens; // is the number of tokens found, this is not always
  uint8_t order_type;
  // the number of all tokens include in the Tx
  /** token1 is often the input token */
  uint8_t token1_address[ADDRESS_LENGTH];
  uint8_t token1_amount[INT256_LENGTH];
  uint8_t token1_decimals;
  char token1_ticker[MAX_TICKER_LEN];
  /** token2 is the output token */
  uint8_t token2_address[ADDRESS_LENGTH];
  uint8_t token2_amount[INT256_LENGTH];
  uint8_t token2_decimals;
  char token2_ticker[MAX_TICKER_LEN];

  // uint8_t nft_id[INT256_LENGTH];
  // uint8_t ui_selector;      // ui_selector is the byte set by SeaPort front
  // to determine the action
  selector_t selectorIndex; // method id
  uint8_t booleans;         // bitwise booleans
  uint8_t skip;             // number of parameters to skip
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
