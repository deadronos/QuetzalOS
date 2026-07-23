#!/bin/bash
set -e

# Include Homebrew path on macOS
export PATH="/opt/homebrew/bin:/usr/local/bin:$PATH"

# Run QuetzalOS in QEMU x86_64 with VESA graphics enabled
qemu-system-x86_64 \
    -kernel quetzal.bin \
    -vga std \
    -m 512M \
    -no-reboot \
    -no-shutdown \
    -serial stdio
