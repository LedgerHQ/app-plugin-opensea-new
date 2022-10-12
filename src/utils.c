#include "eth_internals.h"

uint8_t add_uint256(uint8_t *a, uint8_t *b) {
  uint8_t carry = 0;
  for (uint8_t i = INT256_LENGTH - 1; i > 0; i--) {
    uint16_t added = a[i] + b[i] + carry;
    a[i] += b[i] + carry;
    carry = (added > 255) ? 1 : 0;
  }
  uint16_t added = a[0] + b[0] + carry;
  if (added > 255)
    return 1;
  a[0] += b[0] + carry;
  return 0;
}

uint8_t sub_uint256(uint8_t *a, uint8_t *b) {
  uint8_t carry = 0;
  // for (uint8_t i = INT256_LENGTH - 1; i > 0; i--) {
  for (uint8_t i = INT256_LENGTH - 1; i > 0; i--) {
    int16_t substract = a[i] - b[i] - carry;
    // a[i] = (substract < 0) ? 256 + substract : substract;
    a[i] = substract + ((substract < 0) ? 256 : 0);
    carry = (substract < 0) ? 1 : 0;
  }
  // TODO PENZO check extremes
  int substract = a[0] - b[0] - carry;
  if (substract < 0)
    return 1;
  a[0] -= b[0] + carry;
  return 0;
}
