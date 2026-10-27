#!/usr/bin/env bash
# Print CPU package power in watts, averaged over N seconds (default 1). Needs root.

interval="${1:-1}"

# Find the RAPL package zone by its name, not by its number.
zone=""
for z in /sys/class/powercap/intel-rapl:*; do
    if [ "$(cat "$z/name")" = "package-0" ]; then
        zone="$z"
        break
    fi
done
if [ -z "$zone" ]; then
    echo "error: no RAPL package-0 zone found" >&2
    exit 1
fi

max=$(cat "$zone/max_energy_range_uj")

e1=$(cat "$zone/energy_uj") || exit 1
t1=$(date +%s%N)
sleep "$interval"
e2=$(cat "$zone/energy_uj") || exit 1
t2=$(date +%s%N)

delta=$(( e2 - e1 ))
if [ "$delta" -lt 0 ]; then
    delta=$(( delta + max ))    # the counter wrapped around between the two reads
fi

# Bash can only do whole numbers, so awk does the decimal division.
awk -v e="$delta" -v ns=$(( t2 - t1 )) 'BEGIN { printf "%.2f W\n", (e / 1e6) / (ns / 1e9) }'
