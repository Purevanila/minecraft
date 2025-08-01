#pragma once

#include <array>
#include <cstdint>

namespace UI {
    // Compressed digit patterns using bitfields for efficiency
    constexpr int DIGIT_WIDTH = 5;
    constexpr int DIGIT_HEIGHT = 7;
    constexpr int DIGIT_PIXELS = DIGIT_WIDTH * DIGIT_HEIGHT;
    
    // Each digit pattern stored as a 64-bit integer (35 bits used)
    constexpr std::array<uint64_t, 10> DIGIT_PATTERNS = {{
        // 0: 11111 10001 10001 10001 10001 10001 11111
        0b1111110001100011000110001100011111,
        
        // 1: 00100 01100 00100 00100 00100 00100 01110
        0b0010001100001000010000100001000111,
        
        // 2: 11111 00001 00001 11111 10000 10000 11111
        0b1111100001000011111100001000011111,
        
        // 3: 11111 00001 00001 11111 00001 00001 11111
        0b1111100001000011111000010000111111,
        
        // 4: 10001 10001 10001 11111 00001 00001 00001
        0b1000110001100011111100001000010001,
        
        // 5: 11111 10000 10000 11111 00001 00001 11111
        0b1111110000100001111100001000011111,
        
        // 6: 11111 10000 10000 11111 10001 10001 11111
        0b1111110000100001111110001100011111,
        
        // 7: 11111 00001 00001 00010 00100 01000 10000
        0b1111100001000010001000100010010000,
        
        // 8: 11111 10001 10001 11111 10001 10001 11111
        0b1111110001100011111110001100011111,
        
        // 9: 11111 10001 10001 11111 00001 00001 11111
        0b1111110001100011111100001000011111
    }};
    
    // Extract pixel from compressed pattern
    constexpr bool getDigitPixel(int digit, int x, int y) {
        if (digit < 0 || digit > 9 || x < 0 || x >= DIGIT_WIDTH || y < 0 || y >= DIGIT_HEIGHT) {
            return false;
        }
        int bitIndex = y * DIGIT_WIDTH + x;
        return (DIGIT_PATTERNS[digit] >> (DIGIT_PIXELS - 1 - bitIndex)) & 1;
    }
}
