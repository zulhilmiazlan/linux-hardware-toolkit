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

uint32_t get_field(uint32_t v, unsigned lo, unsigned width) {
    return (v >> lo) & ((1u << width) - 1u);
}

uint32_t set_field(uint32_t v, unsigned lo, unsigned width, uint32_t value) {
    uint32_t ones = (1u << width) - 1u;
    assert(value <= ones);  // value must fit in the field
    return (v & ~(ones << lo)) | ((value & ones) << lo);
}

unsigned popcount(uint32_t v) {
    unsigned count = 0;
    for (unsigned n = 0; n < 32; n++) {   // uint32_t always has exactly 32 bits
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
    assert(get_field(set_field(0x35, 3, 3, 5), 3, 3) == 5);  // write then read back gives the same value

    assert(popcount(0x35) == 4);
    assert(popcount(0) == 0);            // no bits on
    assert(popcount(0xFFFFFFFF) == 32);   // every bit on
    assert(popcount(0x80000000) == 1);   // only the very top bit, switch 31

    assert(bswap32(0x12345678) == 0x78563412);
    assert(bswap32(0x000000FF) == 0xFF000000);
    assert(bswap32(0xAABBBBAA) == 0xAABBBBAA);
    assert(bswap32(bswap32(0x12345678)) == 0x12345678);


    printf("all test passed\n");
    return 0;
}
