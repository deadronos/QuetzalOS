# QuetzalOS Phase 3 Implementation Plan

> **Branch:** `feature/improvements`  
> **Phase:** 3 (Physical Memory Management & Hardware Diagnostics)  
> **Status:** Draft / Pending Review  

This document details the technical plan for **Phase 3**, implementing physical memory allocation driven by the Multiboot memory map, a VGA text-mode fallback driver, and serial port logging (`kprintf`).

---

## Technical Specifications & Planned Changes

### 1. Multiboot Physical Memory Frame Allocator
* **Target:** `kernel/mm/phys.h` (NEW), `kernel/mm/phys.c` (NEW)
* **Design:**
  * Parse Multiboot `mmap_addr` and `mmap_length` structures (`multiboot_memory_map_t`).
  * Identify available RAM regions (`type == 1`) below 4 GB identity map limit.
  * Reserve kernel image, page tables, stack, and framebuffers.
  * Build page frame bitmap tracking 4 KB physical pages.
  * API: `phys_init`, `phys_alloc_page`, `phys_free_page`, `phys_get_free_memory`.

---

### 2. VGA Text-Mode Diagnostic Fallback (`0xB8000`)
* **Target:** `kernel/drivers/vga_text.h` (NEW), `kernel/drivers/vga_text.c` (NEW)
* **Design:**
  * Write directly to 80x25 text-mode video memory at `0xB8000`.
  * Support auto-scrolling, newline formatting, and color attributes.
  * Provides early panic screen and diagnostic output if VBE graphics fails.

---

### 3. Serial Port COM1 (`0x3F8`) UART Driver & `kprintf`
* **Target:** `kernel/arch/x86_64/serial.h` (NEW), `kernel/arch/x86_64/serial.c` (NEW)
* **Design:**
  * Initialize 16550 UART on COM1 (`0x3F8`) with 115200 baud rate.
  * Implement `serial_putc`, `serial_puts`, and formatted `kprintf` logger.
  * Pipes kernel logs directly to QEMU `-serial stdio`.

---

## File Addition Summary

| File Path | Description |
|---|---|
| [kernel/mm/phys.h](file:///Users/openclaw/Github/QuetzalOS/kernel/mm/phys.h) | Physical page allocator declarations |
| [kernel/mm/phys.c](file:///Users/openclaw/Github/QuetzalOS/kernel/mm/phys.c) | Multiboot mmap parser & bitmap frame allocator |
| [kernel/drivers/vga_text.h](file:///Users/openclaw/Github/QuetzalOS/kernel/drivers/vga_text.h) | 80x25 text mode driver header |
| [kernel/drivers/vga_text.c](file:///Users/openclaw/Github/QuetzalOS/kernel/drivers/vga_text.c) | `0xB8000` text mode console implementation |
| [kernel/arch/x86_64/serial.h](file:///Users/openclaw/Github/QuetzalOS/kernel/arch/x86_64/serial.h) | UART COM1 serial logger header |
| [kernel/arch/x86_64/serial.c](file:///Users/openclaw/Github/QuetzalOS/kernel/arch/x86_64/serial.c) | 16550 UART driver & `kprintf` output |
| [Makefile](file:///Users/openclaw/Github/QuetzalOS/Makefile) | Updated build targets for new components |

---

## Verification Plan

### Automated Build Verification
* Run `make clean && make` to verify compilation across all new modules.

### Runtime Verification
* Execute `./qemu.sh` in QEMU.
* Verify serial log output in terminal via COM1 `-serial stdio`.
* Verify physical page allocation stats logged on boot.
