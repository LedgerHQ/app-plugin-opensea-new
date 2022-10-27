#include "eth_internals.h"
#include "seaport_plugin.h"
#include <stdint.h>

/*
**  uint256 utils
*/

/* add b in a
** return 1 on overflow
** return 0 Ok
*/
uint8_t add_uint256(uint8_t *a, uint8_t *b) {
    PRINTF("ADD256 before\ta:\t%.*H\n", INT256_LENGTH, a);
    PRINTF("ADD256 before\tb:\t%.*H\n", INT256_LENGTH, b);
    uint8_t carry = 0;
    for (uint8_t i = INT256_LENGTH - 1; i > 0; i--) {
        uint16_t added = a[i] + b[i] + carry;
        a[i] += b[i] + carry;
        carry = (added > 255) ? 1 : 0;
    }
    uint16_t added = a[0] + b[0] + carry;
    if (added > 255) return 1;
    a[0] += b[0] + carry;
    PRINTF("ADD256 after\ta:\t%.*H\n", INT256_LENGTH, a);
    PRINTF("ADD256 after\tb:\t%.*H\n", INT256_LENGTH, b);
    return 0;
}

/* sub b to a
** return 1 on overflow
** return 0 Ok
*/
uint8_t sub_uint256(uint8_t *a, uint8_t *b) {
    uint8_t carry = 0;
    for (uint8_t i = INT256_LENGTH - 1; i > 0; i--) {
        int16_t substract = a[i] - b[i] - carry;
        a[i] = substract + ((substract < 0) ? 256 : 0);
        carry = (substract < 0) ? 1 : 0;
    }
    int substract = a[0] - b[0] - carry;
    if (substract < 0) return 1;
    a[0] -= b[0] + carry;
    return 0;
}

/*
**  numerator/denominator utils
*/

/* check if number in parameter fit in uint size */
uint8_t does_number_fit(const uint8_t *parameter, uint8_t parameter_length, uint8_t size) {
    for (uint8_t i = 0; i < parameter_length - size; i++) {
        if (parameter[i] != 0) return 1;
    }
    return 0;
}

/* Calculate the number of nfts if numerator is different than denominator
** return 1 on overflow
** return 2 on order will revert
** return 0 Ok
*/
uint8_t calc_number_of_nfts(uint8_t *amount,
                            uint32_t numerator,
                            uint32_t denominator,
                            uint16_t *number_of_nfts) {
    // TODO number_of_nfts should be same size as value, check all sizes
    PRINTF("NB_NFT numerator: %d\n", numerator);
    PRINTF("NB_NFT denominator: %d\n", denominator);
    PRINTF("NB_NFT amount: %.*H\n", INT256_LENGTH, amount);
    PRINTF("NB_NFT number_of_nfts: %d\n", *number_of_nfts);
    PRINTF("NB_NFT --------\n");

    uint32_t value = 0;
    if (does_number_fit(amount, PARAMETER_LENGTH, sizeof(value))) return 1;
    value = U4BE(amount, INT256_LENGTH - sizeof(value));
    PRINTF("NB_NFT value: %d\n", value);
    if (numerator == denominator) {
        (*number_of_nfts) += value;
        if ((*number_of_nfts) < value) return 1;  // overflow
        return 0;
    }
    // Multiply the numerator by the value and ensure no overflow occurs.
    uint64_t valueTimesNumerator = value * numerator;
    if (valueTimesNumerator > UINT32_MAX) return 1;
    uint32_t newValue = valueTimesNumerator / denominator;

    // Ensure that division gave a final result with no remainder.
    // uint32_t exact = ((newValue * denominator) / numerator) == value;
    // if (!exact) revert
    if (!(((newValue * denominator) / numerator) == value)) return 2;
    // Add this order's number_of_nfts to global number_of_nfts.
    PRINTF("NB_NFT newValue: %d\n", newValue);
    PRINTF("NB_NFT number_of_nfts before: %d\n", *number_of_nfts);
    (*number_of_nfts) += newValue;
    PRINTF("NB_NFT number_of_nfts after: %d\n", *number_of_nfts);
    if ((*number_of_nfts) < newValue) return 1;  // overflow
    return 0;
}

/*
** swap token1 and token2 pointer's
*/

void swap_tokens(context_t *context) {
    PRINTF("\n\nSwaping tokens.\n\n");
    token_t save;
    save = context->token1;
    context->token1 = context->token2;
    context->token2 = save;

    // PRINTF("\nbefore swap:\nITEM1_FOUND: %d\nITEM2_FOUND: %d\n",
    //        (context->booleans & ITEM1_FOUND) ? 1 : 0,
    //        (context->booleans & ITEM2_FOUND) ? 1 : 0);

    // // swap tokens found booleans
    // if (context->booleans & ITEM1_FOUND) {
    //     if (!(context->booleans & ITEM2_FOUND)) {
    //         // down ITEM1, up ITEM2
    //         context->booleans ^= ITEM1_FOUND;
    //         context->booleans |= ITEM2_FOUND;
    //     }
    // } else {
    //     if (context->booleans & ITEM2_FOUND) {
    //         // up ITEM1, down ITEM2
    //         context->booleans |= ITEM1_FOUND;
    //         context->booleans ^= ITEM2_FOUND;
    //     }
    // }

    // PRINTF("\nafter swap:\nITEM1_FOUND: %d\nITEM2_FOUND: %d\n",
    //        (context->booleans & ITEM1_FOUND) ? 1 : 0,
    //        (context->booleans & ITEM2_FOUND) ? 1 : 0);
}
