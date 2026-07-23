# Contributing to QuetzalOS

Thank you for contributing to QuetzalOS! QuetzalOS (TonalOS) is a monoprogrammatic, 64-bit flat binary operating system environment designed for high-performance direct hardware interaction in Ring 0.

---

## 🛠️ Build & Test Setup

### Prerequisites
* `x86_64-elf-gcc` cross-compiler (or system GCC with `-m64 -ffreestanding`)
* `nasm` assembler
* `make`
* `qemu-system-x86_64` (for local running & debugging)

### Commands
```bash
# Clean and build the kernel binary
make clean && make

# Launch kernel in QEMU
./qemu.sh
```

---

## 🛡️ Mandatory Adversarial Self-Review

Before submitting a Pull Request, contributors must conduct an **Adversarial Code Review** auditing their changes against these vulnerability classes:

1. **Concurrency & ISR Safety:** Verify `_Atomic` storage and acquire/release ordering for data shared between IRQs and main loops.
2. **Arithmetic & Bounds:** Guard unsigned coordinate subtractions (`cy - height`), check for integer overflow in line algorithms, and enforce framebuffer boundary checks.
3. **Hardware Compatibility:** Validate Multiboot RGB parameters (`type == 1`, `bpp == 32`), 4 GB identity paging limits, and PCI decoding.
4. **Memory Layout Integrity:** Synchronize assembly symbols (`pml4_table`) with header definitions (`memory_map.h`) and ensure stack isolation.

---

## 📜 Code Style Guidelines

* **Standard:** Freestanding C99 (`-std=c99`).
* **Headers:** Prefer freestanding headers (`<stdint.h>`, `<stdatomic.h>`, `<stddef.h>`).
* **Warnings:** Code must compile cleanly with `-Wall -Wextra`.
