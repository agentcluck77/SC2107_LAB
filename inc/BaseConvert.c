/*
 * BaseConvert.c
 *
 *  Created on: 28 Oct 2025
 *      Author: aloy
 */

// BaseConvert.c
// Implementation of number base conversion functions
// No external conversion libraries used

/*
// Example usage of BaseConvert functions
#include "msp.h"
#include "../inc/BaseConvert.h"
#include "../inc/UART0.h"
#include "../inc/Clock.h"

// Helper function to print conversion results
void PrintResult(char* label, int8_t status, char* output) {
    UART0_OutString(label);
    UART0_OutString(": ");

    if (status == CONVERT_SUCCESS) {
        UART0_OutString(output);
    } else {
        UART0_OutString("ERROR ");
        UART0_OutUDec(status);
    }
    UART0_OutString("\r\n");
}

int main(void) {
    uint32_t decimal_result;
    char output_buffer[BIN_BUFFER_SIZE];
    int8_t status;

    // Initialize system
    Clock_Init48MHz();
    UART0_Init();  // Use this for direct UART functions
    // UART0_Initprintf();  // OR use this if you want printf() support

    UART0_OutString("\r\n=== Base Conversion Examples ===\r\n\r\n");

    //----------Binary to Decimal----------
    UART0_OutString("--- Binary to Decimal ---\r\n");

    status = BinToDec("1010", &decimal_result);
    if (status == CONVERT_SUCCESS) {
        UART0_OutString("Binary 1010 = Decimal ");
        UART0_OutUDec(decimal_result);
        UART0_OutString("\r\n");
    }

    status = BinToDec("11111111", &decimal_result);
    if (status == CONVERT_SUCCESS) {
        UART0_OutString("Binary 11111111 = Decimal ");
        UART0_OutUDec(decimal_result);
        UART0_OutString("\r\n");
    }

    //----------Decimal to Binary----------
    UART0_OutString("\r\n--- Decimal to Binary ---\r\n");

    status = DecToBin(10, output_buffer, BIN_BUFFER_SIZE);
    PrintResult("Decimal 10", status, output_buffer);

    status = DecToBin(255, output_buffer, BIN_BUFFER_SIZE);
    PrintResult("Decimal 255", status, output_buffer);

    status = DecToBin(42, output_buffer, BIN_BUFFER_SIZE);
    PrintResult("Decimal 42", status, output_buffer);

    //----------Hexadecimal to Decimal----------
    UART0_OutString("\r\n--- Hexadecimal to Decimal ---\r\n");

    status = HexToDec("FF", &decimal_result);
    if (status == CONVERT_SUCCESS) {
        UART0_OutString("Hex FF = Decimal ");
        UART0_OutUDec(decimal_result);
        UART0_OutString("\r\n");
    }

    status = HexToDec("0x1A5F", &decimal_result);  // With 0x prefix
    if (status == CONVERT_SUCCESS) {
        UART0_OutString("Hex 0x1A5F = Decimal ");
        UART0_OutUDec(decimal_result);
        UART0_OutString("\r\n");
    }

    status = HexToDec("CAFE", &decimal_result);
    if (status == CONVERT_SUCCESS) {
        UART0_OutString("Hex CAFE = Decimal ");
        UART0_OutUDec(decimal_result);
        UART0_OutString("\r\n");
    }

    //----------Decimal to Hexadecimal----------
    UART0_OutString("\r\n--- Decimal to Hexadecimal ---\r\n");

    status = DecToHex(255, output_buffer, HEX_BUFFER_SIZE);
    PrintResult("Decimal 255", status, output_buffer);

    status = DecToHex(6751, output_buffer, HEX_BUFFER_SIZE);
    PrintResult("Decimal 6751", status, output_buffer);

    status = DecToHex(51966, output_buffer, HEX_BUFFER_SIZE);
    PrintResult("Decimal 51966", status, output_buffer);

    //----------Binary to Hexadecimal----------
    UART0_OutString("\r\n--- Binary to Hexadecimal ---\r\n");

    status = BinToHex("11111111", output_buffer, HEX_BUFFER_SIZE);
    PrintResult("Binary 11111111", status, output_buffer);

    status = BinToHex("101010101111", output_buffer, HEX_BUFFER_SIZE);
    PrintResult("Binary 101010101111", status, output_buffer);

    status = BinToHex("1100101011111110", output_buffer, HEX_BUFFER_SIZE);
    PrintResult("Binary 1100101011111110", status, output_buffer);

    //----------Hexadecimal to Binary----------
    UART0_OutString("\r\n--- Hexadecimal to Binary ---\r\n");

    status = HexToBin("FF", output_buffer, BIN_BUFFER_SIZE);
    PrintResult("Hex FF", status, output_buffer);

    status = HexToBin("A5", output_buffer, BIN_BUFFER_SIZE);
    PrintResult("Hex A5", status, output_buffer);

    status = HexToBin("0x1234", output_buffer, BIN_BUFFER_SIZE);
    PrintResult("Hex 0x1234", status, output_buffer);

    UART0_OutString("\r\n=== All conversions complete ===\r\n");

    while(1) {
        // Main loop - could add interactive conversion here
    }
}
 */

#include "BaseConvert.h"
#include <stdint.h>

//------------Helper Functions------------

int8_t char_to_digit(char c) {
    if (c >= '0' && c <= '9') {
        return c - '0';  // '0'->0, '1'->1, ..., '9'->9
    }
    else if (c >= 'A' && c <= 'F') {
        return c - 'A' + 10;  // 'A'->10, 'B'->11, ..., 'F'->15
    }
    else if (c >= 'a' && c <= 'f') {
        return c - 'a' + 10;  // 'a'->10, 'b'->11, ..., 'f'->15
    }
    return -1;  // Invalid character
}

char digit_to_char(uint8_t digit) {
    if (digit <= 9) {
        return '0' + digit;  // 0->'0', 1->'1', ..., 9->'9'
    }
    else if (digit <= 15) {
        return 'A' + (digit - 10);  // 10->'A', 11->'B', ..., 15->'F'
    }
    return '?';  // Invalid digit
}

//------------Core Conversion Functions------------

int8_t BaseToDecimal(const char* str, uint8_t base, uint32_t* result) {
    uint32_t value = 0;
    int8_t digit;
    const char* ptr = str;

    // Validate inputs
    if (str == 0 || result == 0) {
        return CONVERT_ERROR_NULL;
    }

    if (base != 2 && base != 10 && base != 16) {
        return CONVERT_ERROR_BASE;
    }

    // Skip optional "0x" or "0X" prefix for hex
    if (base == 16 && ptr[0] == '0' && (ptr[1] == 'x' || ptr[1] == 'X')) {
        ptr += 2;
    }

    // Process each character
    while (*ptr != '\0') {
        digit = char_to_digit(*ptr);

        // Check if digit is valid for this base
        if (digit < 0 || digit >= base) {
            return CONVERT_ERROR_INVALID;
        }

        // Check for overflow before multiplying
        if (value > (0xFFFFFFFF / base)) {
            return CONVERT_ERROR_OVERFLOW;
        }

        value = value * base;

        // Check for overflow before adding
        if (value > (0xFFFFFFFF - digit)) {
            return CONVERT_ERROR_OVERFLOW;
        }

        value = value + digit;
        ptr++;
    }

    *result = value;
    return CONVERT_SUCCESS;
}

int8_t DecimalToBase(uint32_t num, uint8_t base, char* output, uint8_t buffer_size) {
    uint8_t i = 0;
    uint8_t start, end;
    char temp;

    // Validate inputs
    if (output == 0) {
        return CONVERT_ERROR_NULL;
    }

    if (base != 2 && base != 10 && base != 16) {
        return CONVERT_ERROR_BASE;
    }

    // Check minimum buffer size
    if (buffer_size < 2) {  // Need at least space for 1 digit + null
        return CONVERT_ERROR_OVERFLOW;
    }

    // Special case: if num is 0
    if (num == 0) {
        output[0] = '0';
        output[1] = '\0';
        return CONVERT_SUCCESS;
    }

    // Extract digits (they come out in reverse order)
    while (num > 0 && i < (buffer_size - 1)) {
        output[i] = digit_to_char(num % base);
        num = num / base;
        i++;
    }

    // Check if we ran out of buffer space
    if (num > 0) {
        return CONVERT_ERROR_OVERFLOW;
    }

    // Null terminate
    output[i] = '\0';

    // Reverse the string (digits were extracted backwards)
    start = 0;
    end = i - 1;
    while (start < end) {
        temp = output[start];
        output[start] = output[end];
        output[end] = temp;
        start++;
        end--;
    }

    return CONVERT_SUCCESS;
}

//------------Convenience Wrapper Functions------------

int8_t BinToDec(const char* bin_str, uint32_t* result) {
    return BaseToDecimal(bin_str, 2, result);
}

int8_t DecToBin(uint32_t decimal, char* output, uint8_t buffer_size) {
    return DecimalToBase(decimal, 2, output, buffer_size);
}

int8_t HexToDec(const char* hex_str, uint32_t* result) {
    return BaseToDecimal(hex_str, 16, result);
}

int8_t DecToHex(uint32_t decimal, char* output, uint8_t buffer_size) {
    return DecimalToBase(decimal, 16, output, buffer_size);
}

int8_t BinToHex(const char* bin_str, char* output, uint8_t buffer_size) {
    uint32_t decimal;
    int8_t status;

    // First convert binary to decimal
    status = BaseToDecimal(bin_str, 2, &decimal);
    if (status != CONVERT_SUCCESS) {
        return status;
    }

    // Then convert decimal to hex
    return DecimalToBase(decimal, 16, output, buffer_size);
}

int8_t HexToBin(const char* hex_str, char* output, uint8_t buffer_size) {
    uint32_t decimal;
    int8_t status;

    // First convert hex to decimal
    status = BaseToDecimal(hex_str, 16, &decimal);
    if (status != CONVERT_SUCCESS) {
        return status;
    }

    // Then convert decimal to binary
    return DecimalToBase(decimal, 2, output, buffer_size);
}


