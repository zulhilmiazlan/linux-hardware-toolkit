#!/usr/bin/env bash
# Capture this machine's hardware and software state into a timestamped folder.

host=$(uname -n)
stamp=$(date +%Y%m%d-%H%M%S)
outdir="state_${host}_${stamp}"

mkdir -p "$outdir"

manifest="$outdir/manifest.txt"

{
    echo "capture_state.sh version 1"
    echo "host: $host"
    echo "date: $(date --iso-8601=seconds)"
    echo "user: $(id -un)"
    if [ "$EUID" -eq 0 ]; then
        echo "root: yes"
    else
        echo "root: no (items that need sudo will fail)"
    fi
    echo
} > "$manifest"


# capture NAME COMMAND...: run COMMAND, save its output to NAME, record what happened.
capture() {
    local name="$1"
    shift

    if ! command -v "$1" > /dev/null; then
        echo "SKIPPED $name: '$1' is not installed" >> "$manifest"
        return
    fi

    if "$@" > "$outdir/$name" 2>&1; then
        echo "ok      $name: $*" >> "$manifest"
    else
        echo "FAILED  $name: $* (exit code $?)" >> "$manifest"
    fi
}

# Kernel and OS
capture uname.txt        uname -a
capture os-release.txt   cat /etc/os-release
capture cmdline.txt      cat /proc/cmdline

# CPU and memory
capture lscpu.txt        lscpu
capture cpuinfo.txt      cat /proc/cpuinfo
capture meminfo.txt      cat /proc/meminfo

# Firmware: BIOS version and board
capture bios.txt         grep . /sys/class/dmi/id/bios_version /sys/class/dmi/id/bios_date /sys/class/dmi/id/board_name /sys/class/dmi/id/product_name

# Devices
capture lspci.txt        lspci -vvv
capture lspci-tree.txt   lspci -tv
capture lsusb.txt        lsusb -t
capture lsblk.txt        lsblk -o NAME,SIZE,MODEL,TRAN

# Kernel log
capture dmesg.txt        dmesg

echo "Saved to $outdir"
