CC     = gcc
CFLAGS = -std=c17 -Wall -Wextra -Wpedantic -Werror -g

hd: hd.c
	$(CC) $(CFLAGS) hd.c -o hd

test: hd
	./run_tests.sh

.PHONY: test
