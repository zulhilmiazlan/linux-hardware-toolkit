#include <stdio.h>
#include <stdlib.h>

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
    size_t offset = 0;  // position in the file
    size_t n;

    while ((n = fread(line, 1, 16, f)) > 0) {
        printf("%08zx ", offset);
        for (size_t i = 0; i < n; i++) {
            printf(" %02x", line[i]);
        }
        printf("\n");
        offset += n;
    }

    fclose(f);
    return 0;
}
