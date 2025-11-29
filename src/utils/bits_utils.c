#include "bits_utils.h"
#include "../defs.h"
#include <assert.h>
// #include "machine.h"

u64 extract_bits_u64(u64 source, int start, int end) {
  assert(start <= end); // PRE: start <= end
  return ((source >> start) & ((1ULL << (end - start + 1)) - 1ULL));
}

bool check_bit_u64(u64 source, int index) {
  return (source & (1ULL << index)) && (index < 64);
}

u32 extract_bits_u32(u32 source, int start, int end) {
  assert(start <= end); // PRE: start <= end
  return ((source >> start) & ((1U << (end - start + 1)) - 1U));
}

bool check_bit_u32(u32 source, int index) {
  return (source & (1U << index)) && (index < 32);
}

void insert_bits_u64(u64 *target, int start, int end, u64 value) {
  u64 mask = ((1ULL << (end - start + 1)) - 1ULL) << start;
  *target = ((*target) & ~mask) | ((value << start) & mask);
}

void insert_bits_u32(u32 *target, int start, int end, u32 value) {
  u32 mask = ((1U << (end - start + 1)) - 1U) << start;
  *target = ((*target) & ~mask) | ((value << start) & mask);
}

i64 sign_extend(u64 value, int original_bits) {
  assert(original_bits > 0 &&
         original_bits <= 64); // PRE: 0 < original_bits <= 64
  u64 mask = 1ULL << (original_bits - 1);
  return (value ^ mask) - mask;
}

void write_u32_le(FILE *out, u32 word) {
  fputc(word & 0xFF, out);
  fputc((word >> 8) & 0xFF, out);
  fputc((word >> 16) & 0xFF, out);
  fputc((word >> 24) & 0xFF, out);
}
