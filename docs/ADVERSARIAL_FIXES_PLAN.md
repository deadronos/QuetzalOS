# QuetzalOS Adversarial Findings Remediation Plan

> **Branch:** `feature/improvements`  
> **Status:** Draft / Pending Review  

This document outlines the detailed remediation plan for the 5 critical security, concurrency, hardware compatibility, and memory safety findings identified during the adversarial code review.

---

## Technical Specifications & Planned Changes

### 1. PS/2 Extended Key State Machine & Shift Support
* **Target:** `kernel/drivers/ps2_keyboard.c`, `kernel/drivers/ps2_keyboard.h`
* **Changes:**
  * Add static state `static int is_e0 = 0;` and `static int shift_pressed = 0;`.
  * Track Left Shift (`0x2A`) and Right Shift (`0x36`) press/release events.
  * When `0xE0` is received, toggle `is_e0 = 1` and defer processing to the next scancode byte.
  * Map `0xE0 0x48` (Up Arrow), `0xE0 0x50` (Down Arrow), `0xE0 0x4B` (Left Arrow), `0xE0 0x4D` (Right Arrow) or process shifted ASCII mapping table (`scancode_ascii_shift`).

---

### 2. Multiboot Framebuffer Type Validation
* **Target:** `kernel/kernel.c`, `kernel/drivers/vbe.h`
* **Changes:**
  * Verify `mb_info->framebuffer_type == 1` (Direct RGB) and `mb_info->framebuffer_bpp == 32` before accepting Multiboot-provided framebuffer params.
  * If `framebuffer_type != 1` (e.g. EGA text mode `type = 2`), fall back to PCI LFB probing and Bochs DISPI mode initialization.

---

### 3. Line Draw & String Clipping Safeguards
* **Target:** `kernel/drivers/vbe.c`
* **Changes:**
  * In `vbe_draw_line`: Clamp `x0, x1` to `[0, fb_w - 1]` and `y0, y1` to `[0, fb_h - 1]` before casting to `int` and running Bresenham delta calculations.
  * In `vbe_draw_string`: Auto-wrap text onto newline (`cx = x; cy += 10;`) when `cx + 8 >= fb_w`.

---

### 4. Page Table Address Symbol Alignment
* **Target:** `boot/boot.asm`, `kernel/memory_map.h`
* **Changes:**
  * Declare `global pml4_table`, `global pdpt_table`, `global pd_tables` in `boot/boot.asm`.
  * In `kernel/memory_map.h`, update `QOS_PML4_ADDR`, `QOS_PDPT_ADDR`, and `QOS_PD_BASE` to reference `(uintptr_t)&pml4_table` symbols rather than hardcoded offsets desynchronized from the linker.

---

### 5. Boot Stack Section Isolation
* **Target:** `boot/boot.asm`
* **Changes:**
  * Reorder `.bss` allocations so page tables (`pml4_table`, `pdpt_table`, `pd_tables`) are placed *before* `stack_bottom` / `stack_top`, ensuring stack growth downwards does not overwrite active page table entries.

---

## Verification Plan

### Automated Build Verification
* Run `make clean && make` to verify compilation and linking with zero warnings.

### Runtime Verification
* **QEMU Test:** Boot kernel image with `./qemu.sh`.
* **Keyboard Test:** Test Shift key typing (uppercase letters, numbers to symbols) and Arrow keys.
* **Text Wrapping Test:** Pass long string to `vbe_draw_string` and verify clean soft-wrapping at screen edge without memory overflow.
