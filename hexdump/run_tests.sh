#!/usr/bin/env bash
# Compare ./hd with hexdump -C on every file in tests/.

passed=0
failed=0

for f in tests/*; do
    if [ ! -f "$f" ]; then
        echo "FAIL $f (test file missing)"
        failed=$((failed + 1))
        continue
    fi

    hd_out=$(./hd "$f")
    hd_status=$?
    ref_out=$(hexdump -C "$f")
    ref_status=$?

    if [ "$hd_status" -ne 0 ] || [ "$ref_status" -ne 0 ]; then
        echo "FAIL $f (exit codes: hd=$hd_status, hexdump=$ref_status)"
        failed=$((failed + 1))
    elif [ "$hd_out" = "$ref_out" ]; then
        echo "PASS $f"
        passed=$((passed + 1))
    else
        echo "FAIL $f (output differs)"
        failed=$((failed + 1))
    fi
done

echo "$passed passed, $failed failed"
[ "$failed" -eq 0 ]
