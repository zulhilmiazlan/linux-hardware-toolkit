# hd: A Hexdump Clone in C

`hd` is a command-line hexdump utility written in C that displays file contents in hexadecimal and ASCII, with output designed to match `hexdump -C`. It supports offset and length arguments for inspecting specific portions of a file.

## Build and Run

### Build

Requires GCC, GNU Make, and `hexdump` for running the test suite.

```bash
make
```

The Makefile rebuilds `hd` when `hd.c` has changed.

### Usage

```text
./hd [-s OFFSET] [-n LENGTH] FILE
```

- `-s OFFSET` skips the specified number of bytes before displaying the contents.
- `-n LENGTH` limits the number of bytes displayed.
- `FILE` specifies the input file.

Offset and length values can be provided in decimal or hexadecimal using the `0x` prefix. Negative or invalid values are rejected with an error.

### Example

```text
$ ./hd tests/hello.txt
00000000  48 65 6c 6c 6f 2c 20 76  61 6c 69 64 61 74 69 6f  |Hello, validatio|
00000010  6e 21 0a                                          |n!.|
00000013
```

The output shows the byte offset, hexadecimal values, ASCII representation, and final file size.

## Testing

Run the automated test suite:

```bash
make test
```

The suite compares `hd` against `hexdump -C` using seven fixed test files covering different edge cases.

| Test file | What it checks |
|---|---|
| `empty.bin` | Empty input |
| `one.bin` | A single byte and padding |
| `sixteen.bin` | Exactly one full 16-byte line |
| `seventeen.bin` | A partial second line |
| `zeros.bin` | Repeated-line suppression |
| `repeat.bin` | Repeated lines followed by a different line |
| `hello.txt` | Ordinary text input with a newline |

The test script checks that each input file exists, both programs exit successfully, and their outputs match. These checks were added after a test reported `PASS` for a missing file: both programs printed nothing, so their empty outputs matched. The script now rejects that false pass and returns a non-zero exit status if any test fails.

I also deliberately introduced a padding bug to verify that the tests could detect a regression. Three files with partial final lines failed, while the four unaffected cases passed. After restoring the padding, all seven tests passed again.

## Design Notes

### Measuring before optimising

I investigated whether reading input in 16-byte chunks was inefficient and whether manual buffering would improve performance.

Using `time` and `strace`, I measured the program on a 50 MB file. If every 16-byte `fread` operation resulted in a separate kernel read, processing the file would require approximately 3,125,000 read calls. In practice, `strace` recorded only 12,210 calls, consistent with standard I/O buffering data in roughly 4 KB chunks.

On the same file, `hd` took 2.5 seconds, compared with 5.2 seconds for `hexdump -C`, making `hd` approximately twice as fast in that test. Read-system-call overhead accounted for less than 0.1% of the runtime.

Based on these measurements, I decided not to add manual chunking. The evidence did not justify the extra implementation complexity.

This investigation reinforced the importance of measuring actual system behaviour before optimising based on assumptions.

## Known Limitations

- Reading from standard input is not yet supported.
- When `-s OFFSET` exceeds the file size, the program prints the requested skip offset instead of the actual file size.
