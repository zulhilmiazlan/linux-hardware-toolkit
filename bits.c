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

int main (void) {

    assert(set_bit(0x35, 1) == 0x37);
    assert(set_bit(0x35, 3) == 0x3D);
    assert(set_bit(0x35, 0) == 0x35);

    assert(clear_bit(0x35, 1) == 0x35);
    assert(clear_bit(0x35, 5) == 0x15);
    
    assert(test_bit(0x35, 4) == true);
    assert(test_bit(0x35, 3) == false);

    printf("all test passed\n");
    return 0;
}
