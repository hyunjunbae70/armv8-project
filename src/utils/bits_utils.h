#ifndef BITS_UTILS
#define BITS_UTILS

#include <stdio.h>
#include <stdbool.h>
#include "../defs.h"
#include "../emulator/emulate.h"

/**
 * Extracts a closed interval of bits from a signed 64-bit intger.
 * CAREFUL: OUTPUT MIGHT OVERFLOW
 *
 * PRE: start <= end
 * @param source The integer to extract from.
 * @param start  The starting bit index (inclusive).
 * @param end    The ending bit index (inclusive).
 * @return The extracted bits as u64.
 */
u64 extract_bits_u64(u64 source, int start, int end);

/**
 * Checks the value of one bit in a unsigned 64-bit integer.
 *
 * @param source The integer to check.
 * @param index  The bit index to inspect.
 * @return true if the bit is 1, false otherwise.
 */
bool check_bit_u64(u64 source, int index);

/**
 * Extracts a closed interval of bits from a unsigned 32-bit integer.
 *
 * PRE: start <= end
 * @param source The integer to extract from.
 * @param start  The starting bit index (inclusive).
 * @param end    The ending bit index (inclusive).
 * @return The extracted bits as an u32.
 */
u32 extract_bits_u32(u32 source, int start, int end);

/**
 * Checks the value of one bit in a unsigned 32-bit integer.
 *
 * @param source The u32 to check.
 * @param index  The bit index to inspect.
 * @return true if the bit is 1, false otherwise.
 */
bool check_bit_u32(u32 source, int index);

/**
 * Inserts a closed interval of bits into a unsigned 64-bit integer.
 *
 * @param target The integer to insert into.
 * @param start  The starting bit index (inclusive).
 * @param end    The ending bit index (inclusive).
 * @param value  The value to be inserted.
 */
void insert_bits_u64(u64 *target, int start, int end, u64 value);

/**
 * Inserts a closed interval of bits into a unsigned 64-bit integer.
 *
 * @param target The integer to insert into.
 * @param start  The starting bit index (inclusive).
 * @param end    The ending bit index (inclusive).
 * @param value  The value to be inserted.
 */
void insert_bits_u32(u32 *target, int start, int end, u32 value);

/**
 * Sign-extends a given value of any bit width to 64 bits.
 *
 * @param value The value to be extended.
 * @param original_bits  The number of bits in the original value.
 * @return A sign extended 64 bit signed integer.
 */
extern i64 sign_extend(u64 value, int original_bits);

/**
 * Logical shift left by a given number of bits.
 * @param value The value to shift.
 * @param shift_bits  The number of bits to shift.
 * @return The shifted value.
 */
static inline u64 logical_shift_left(u64 value, int shift_bits) {
    return value << shift_bits;
}

/**
 * Clears the upper half of 32 bits of a 64 bit value. Used to right to Wn registers.
 * @param value The value to clear the upper 32 bits off.
 * @return value with upper 32 bits set as zero.
 */
static inline u64 zero_upper_32(u64 value) {
    return (u32)value;
}

/**
 * Checks if the register index points to the zero register.
 * @param reg_index The index of the register in rd/rn. The spec defines the value 11111 to point to the ZR register.
 * @return true if register points to ZR else false.
 */
static inline bool is_ZR(int reg_index) {
    return reg_index == REG_COUNT; 
}

/**
 * Writing a word into a little-endian output file.
 * @param FILE the file in which the word has to be saved.
 * @param word the word to be written.
 */
void write_u32_le(FILE *out, u32 word);

#endif // BITS_UTILS_H
