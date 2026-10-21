#include <stdio.h>
#include <stdlib.h>
#include <ctype.h>
#include <string.h>
#include <stdbool.h>

int main(int argc, char *argv[]) {
    unsigned long skip = 0;
    unsigned long limit = 0;
    bool have_limit = false;
    char *filename = NULL;

    for (int i = 1; i < argc; i++) {
        if (strcmp(argv[i], "-s") == 0) {
            if (!(i + 1 < argc)) {
                fprintf(stderr, "error: -s needs a number\n");
                return 1;
            }
            if (argv[i + 1][0] == '-') {
                fprintf(stderr, "error: -s cannot be negative: %s\n", argv[i + 1]);
                return 1;
            }
            char *end;
            skip = strtoul(argv[i + 1], &end, 0);
            if (end == argv[i + 1] || *end != '\0') {
                fprintf(stderr, "error: bad number for -s: %s\n", argv[i + 1]);
                return 1;
            }
            i++;
        } else if (strcmp(argv[i], "-n") == 0) {
            if (!(i + 1 < argc)) {
                fprintf(stderr, "error: -n needs a number\n");
                return 1;
            }
            if (argv[i + 1][0] == '-') {
                fprintf(stderr, "error: -n cannot be negative: %s\n", argv[i + 1]);
                return 1;
            }
            char *end;
            limit = strtoul(argv[i + 1], &end, 0);
            if (end == argv[i + 1] || *end != '\0') {
                fprintf(stderr, "error: bad number for -n: %s\n", argv[i + 1]);
                return 1;
            }
            have_limit = true;
            i++;
        } else {
            if (filename != NULL) {
                fprintf(stderr, "error: only one file allowed (got %s and %s)\n", filename, argv[i]);
                return 1;
            }
            filename = argv[i];
        }
    }

    if (filename == NULL) {
        fprintf(stderr, "usage: %s [-s OFFSET] [-n LENGTH] FILE\n", argv[0]);
        return 1;
    }

    FILE *f = fopen(filename, "rb");
    if (f == NULL) {
        perror(filename);
        return 1;
    }

    if (fseek(f, skip, SEEK_SET) != 0) {
        perror(filename);
        fclose(f);
        return 1;
    }

    unsigned char line[16];  // one line's worth of bytes
    unsigned char prev[16];  // stores the contents of the previous line
    size_t prev_n = 0;  // stores the length of the previous line
    bool star_printed = false;  // tracking flag for the duplicating sequence
    size_t offset = skip;  // position in the file: starts where -s put us
    size_t n;

    while (true) {
        size_t want = 16;

        if (have_limit) {
            size_t already_read = offset - skip;
            size_t remaining = limit - already_read;

            if (remaining < want) {
                want = remaining;
            }
        }

        if (want == 0) {
            break;
        }

        n = fread(line, 1, want, f);
        if (n == 0) {
            break;
        }

        if (offset > skip && n == prev_n && memcmp(line, prev, n) == 0) {
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
