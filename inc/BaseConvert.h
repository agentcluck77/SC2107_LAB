/*
 * BaseConvert.h
 *
 *  Created on: 28 Oct 2025
 *      Author: aloy
 */

// Number base conversion functions for embedded systems
// No external conversion libraries required
// Supports binary, decimal, and hexadecimal conversions

#ifndef BASECONVERT_H_
#define BASECONVERT_H_

#include <stdint.h>

// Error codes
#define CONVERT_SUCCESS         0
#define CONVERT_ERROR_INVALID   -1  // Invalid character in input
#define CONVERT_ERROR_BASE      -2  // Unsupported base
#define CONVERT_ERROR_OVERFLOW  -3  // Number too large
#define CONVERT_ERROR_NULL      -4  // NULL pointer passed

// Buffer size macros
// 32-bit binary needs max 32 chars + null terminator
#define BIN_BUFFER_SIZE  33
// 32-bit hex needs max 8 chars + null terminator
#define HEX_BUFFER_SIZE  9
// 32-bit decimal needs max 10 chars + null terminator
#define DEC_BUFFER_SIZE  11

//------------Helper Functions------------

// Convert a character '0'-'9', 'A'-'F', 'a'-'f' to its numeric value 0-15
// Input: c - character to convert
// Output: numeric value (0-15), or -1 if invalid
// Examples: '0'->0, '5'->5, 'A'->10, 'F'->15
int8_t char_to_digit(char c);

// Convert a numeric value 0-15 to its character representation
// Input: digit - value from 0 to 15
// Output: character '0'-'9' or 'A'-'F', or '?' if invalid
// Examples: 0->'0', 5->'5', 10->'A', 15->'F'
char digit_to_char(uint8_t digit);

//------------Core Conversion Functions------------

// Convert a string representation in any base to decimal (uint32_t)
// Input: str - null-terminated string (e.g., "1010" for binary, "FF" for hex)
//        base - the base of input string (2, 10, or 16)
//        result - pointer to store the decimal result
// Output: CONVERT_SUCCESS or error code
// Examples:
//   BaseToDecimal("1010", 2, &result) -> result=10
//   BaseToDecimal("FF", 16, &result) -> result=255
//   BaseToDecimal("100", 10, &result) -> result=100
int8_t BaseToDecimal(const char* str, uint8_t base, uint32_t* result);

// Convert a decimal number to string representation in any base
// Input: num - decimal number to convert
//        base - target base (2, 10, or 16)
//        output - buffer to store result string (must be large enough!)
//        buffer_size - size of output buffer
// Output: CONVERT_SUCCESS or error code
// Note: output buffer should be at least BIN_BUFFER_SIZE for binary,
//       HEX_BUFFER_SIZE for hex, DEC_BUFFER_SIZE for decimal
// Examples:
//   DecimalToBase(10, 2, buf, 33) -> buf="1010"
//   DecimalToBase(255, 16, buf, 9) -> buf="FF"
//   DecimalToBase(100, 10, buf, 11) -> buf="100"
int8_t DecimalToBase(uint32_t num, uint8_t base, char* output, uint8_t buffer_size);

//------------Convenience Wrapper Functions------------

// Binary to Decimal
// Input: bin_str - null-terminated binary string (e.g., "11011101")
//        result - pointer to store decimal result
// Output: CONVERT_SUCCESS or error code
// Example: BinToDec("1010", &result) -> result=10
int8_t BinToDec(const char* bin_str, uint32_t* result);

// Decimal to Binary
// Input: decimal - number to convert
//        output - buffer for binary string (min BIN_BUFFER_SIZE bytes)
//        buffer_size - size of output buffer
// Output: CONVERT_SUCCESS or error code
// Example: DecToBin(10, buf, 33) -> buf="1010"
int8_t DecToBin(uint32_t decimal, char* output, uint8_t buffer_size);

// Hexadecimal to Decimal
// Input: hex_str - null-terminated hex string (e.g., "1A5F", can have "0x" prefix)
//        result - pointer to store decimal result
// Output: CONVERT_SUCCESS or error code
// Example: HexToDec("FF", &result) -> result=255
int8_t HexToDec(const char* hex_str, uint32_t* result);

// Decimal to Hexadecimal
// Input: decimal - number to convert
//        output - buffer for hex string (min HEX_BUFFER_SIZE bytes)
//        buffer_size - size of output buffer
// Output: CONVERT_SUCCESS or error code
// Example: DecToHex(255, buf, 9) -> buf="FF"
int8_t DecToHex(uint32_t decimal, char* output, uint8_t buffer_size);

// Binary to Hexadecimal
// Input: bin_str - null-terminated binary string
//        output - buffer for hex string (min HEX_BUFFER_SIZE bytes)
//        buffer_size - size of output buffer
// Output: CONVERT_SUCCESS or error code
// Example: BinToHex("11111111", buf, 9) -> buf="FF"
int8_t BinToHex(const char* bin_str, char* output, uint8_t buffer_size);

// Hexadecimal to Binary
// Input: hex_str - null-terminated hex string (can have "0x" prefix)
//        output - buffer for binary string (min BIN_BUFFER_SIZE bytes)
//        buffer_size - size of output buffer
// Output: CONVERT_SUCCESS or error code
// Example: HexToBin("FF", buf, 33) -> buf="11111111"
int8_t HexToBin(const char* hex_str, char* output, uint8_t buffer_size);

#endif /* BASECONVERT_H_ */
