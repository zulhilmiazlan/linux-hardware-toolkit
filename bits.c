#include <stdint.h>  // uint32_t: an unsigned integer that's exactly 32 bits
#include <stdbool.h> // bool, true, false
#include <assert.h>  // assert()
#include <stdio.h>  // printf()

uint32_t set_bit(uint32_t v, unsigned n) {
    return v | (1u << n);
}

uint32_t clear_bit(uint32_t v, unsigned n) {
    return v & ~(1u << n);
}

bool test_bit(uint32_t v, unsigned n) {
    return v & (1u << n);
}

uint32_t toggle_bit(uint32_t v, unsigned n) {
    return v ^ (1u << n);
}

uint32_t field_ones(unsigned width) {
    assert(width <= 32);
    if (width == 32) {
        return 0xFFFFFFFFu;  // shifting by 32 is undefined in C, so handle it directly
    }
    return (1u << width) - 1u;
}

uint32_t get_field(uint32_t v, unsigned lo, unsigned width) {
    assert(lo < 32);
    assert(lo + width <= 32);
    return (v >> lo) & field_ones(width);
}

uint32_t set_field(uint32_t v, unsigned lo, unsigned width, uint32_t value) {
    assert(lo < 32);
    assert(lo + width <= 32);
    uint32_t ones = field_ones(width);
    assert(value <= ones);
    return (v & ~(ones << lo)) | ((value & ones) << lo);
}

unsigned popcount(uint32_t v) {
    unsigned count = 0;
    for (unsigned n = 0; n < 32; n++) {
        if (test_bit(v, n)) {
            count++;
        }
    }
    return count;
}

uint32_t bswap32(uint32_t v) {
    return (get_field(v, 24, 8) << 0)  |
           (get_field(v, 16, 8) << 8)  |
           (get_field(v, 8,  8) << 16) |
           (get_field(v, 0,  8) << 24);
}

bool is_little_endian(void) {
    uint32_t x = 1;
    unsigned char *first = (unsigned char *)&x;
    return *first == 1;
}

int main (void) {

    assert(set_bit(0x35, 1) == 0x37);
    assert(set_bit(0x35, 3) == 0x3D);
    assert(set_bit(0x35, 0) == 0x35);

    assert(clear_bit(0x35, 1) == 0x35);
    assert(clear_bit(0x35, 5) == 0x15);

    assert(test_bit(0x35, 4) == true);
    assert(test_bit(0x35, 3) == false);

    assert(toggle_bit(0x35, 3) == 0x3D);
    assert(toggle_bit(0x35, 2) == 0x31);
    assert(toggle_bit(toggle_bit(0x35, 4), 4) == 0x35);

    assert(get_field(0x35, 3, 3) == 6);
    assert(get_field(0xF5, 3, 3) == 6);
    assert(get_field(0x357, 4, 8) == 0x35);

    assert(set_field(0x35, 3, 3, 3) == 0x1D);
    assert(set_field(0x35, 3, 3, 6) == 0x35);
    assert(set_field(0x0357, 4, 8, 0xAB) == 0x0AB7);
    assert(get_field(set_field(0x35, 3, 3, 5), 3, 3) == 5);
    assert(popcount(0x35) == 4);
    assert(popcount(0) == 0);
    assert(popcount(0xFFFFFFFF) == 32);
    assert(popcount(0x80000000) == 1);

    assert(bswap32(0x12345678) == 0x78563412);
    assert(bswap32(0x000000FF) == 0xFF000000);
    assert(bswap32(0xAABBBBAA) == 0xAABBBBAA);
    assert(bswap32(bswap32(0x12345678)) == 0x12345678);

    assert(is_little_endian() == true);  // platform assumption: this toolkit expects little-endian

    // edge cases: top bit
    assert(set_bit(0x00000000, 31)    == 0x80000000);
    assert(clear_bit(0xFFFFFFFF, 31)  == 0x7FFFFFFF);
    assert(toggle_bit(0x80000000, 31) == 0x00000000);
    assert(test_bit(0x80000000, 31)   == true);

    // edge cases: field widths
    assert(get_field(0x35, 3, 0) == 0);                      // width 0: reads nothing
    assert(set_field(0x35, 3, 0, 0) == 0x35);                // width 0: writes nothing
    assert(get_field(0x12345678, 0, 32) == 0x12345678);      // width 32: was undefined behaviour before field_ones

    printf("all test passed\n");
    return 0;
}
