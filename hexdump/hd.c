#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

int main(int argc, char *argv[]) {
    if (argc != 2) {
        fprintf(stderr, "usage: %s FILE\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(argv[1], "rb");
    if (f == NULL) {
        perror(argv[1]);
        return 1;
    }

    unsigned char line[16];  // one line's worth of bytes
    unsigned char prev[16];  // stores the contents of the previous line
    size_t prev_n = 0;  // stores the length of the previous line
    bool star_printed = false;  // tracking flag for the duplicating sequence
    size_t offset = 0;  // position in the file
    size_t n;

    while ((n = fread(line, 1, 16, f)) > 0) {
        if (offset > 0 && n == prev_n && memcmp(line, prev, n) == 0) {
            if (!star_printed) {
                printf("*\n");
                star_printed = true;
            }

        } else {
            printf("%08zx ", offset);
            for (size_t i = 0; i < n; i++) {
                if (i == 8) {
                    printf(" ");
                }
                printf(" %02x", line[i]);
            }

            for (size_t i = n; i < 16; i++) {
                if (i == 8) {
                    printf(" ");
                }
                printf("   ");
            }

            printf("  |");

            for (size_t i =0; i < n; i++) {
                if (isprint(line[i])) {
                    putchar(line[i]);
                } else {
                    putchar('.');
                }
            }

            printf("|\n");

            star_printed = false;
        }

        memcpy(prev, line, n);
        prev_n = n;

        offset += n;
    }

    if (offset > 0) {
        printf("%08zx\n", offset);
    }

    fclose(f);
    return 0;
}
