# QuetzalOS Implementation Plan

> **Branch:** `feature/improvements`  
> **Status:** Draft / Pending Review  
> **Created:** 2026-07-23  

This document outlines the phased execution plan for implementing outstanding correctness fixes, core runtime features, memory management, and hardware infrastructure for QuetzalOS based on [docs/IMPROVEMENTS.md](file:///Users/openclaw/Github/QuetzalOS/docs/IMPROVEMENTS.md).

---

## Plan Overview

The implementation is divided into 4 main execution phases:

1. **Phase 1: High-Priority Correctness & Safety (Immediate Target)**
2. **Phase 2: SerpentC Runtime & Interactive Interpreter**
3. **Phase 3: Physical Memory Management & Hardware Diagnostics**
4. **Phase 4: Advanced Drivers, Performance Optimization & CI**

---

## Phase 1: High-Priority Correctness & Safety

Fixes runtime edge cases, torn data reads, integer underflows, and hardware initialization bugs.

### 1.1 Atomic PS/2 Keyboard Ring Buffer (H1)
* **Target:** `kernel/drivers/ps2_keyboard.c`, `kernel/drivers/ps2_keyboard.h`
* **Goal:** Use `<stdatomic.h>` (`_Atomic uint16_t kbd_head`, `kbd_tail`) with `atomic_load_explicit` / `atomic_store_explicit` in the IRQ handler and main reader loop to eliminate torn reads and compiler reordering across interrupt boundaries.

### 1.2 Pyramid & Geometry Bounds Clipping (H2)
* **Target:** `kernel/graphics/ritual_geo.c`, `kernel/drivers/vbe.c`
* **Goal:** Prevent `cy - (i * step_height)` underflows when rendering near the top of the screen. Add explicit clipping bounds to `draw_step_pyramid` and ensure caller coordinates do not write outside the framebuffer boundary.

### 1.3 Multiboot Framebuffer Preservation (H4)
* **Target:** `kernel/drivers/vbe.c`
* **Goal:** Avoid re-issuing Bochs VBE DISPI commands during `vbe_init` if the Multiboot header/info structure indicates the bootloader has already initialized a valid LFB mode.

### 1.4 Named Segment Selector & Gate Constants (M6)
* **Target:** `kernel/arch/x86_64/segments.h` (new), `kernel/arch/x86_64/idt.c`
* **Goal:** Promote magic hex flags (`0x08`, `0x8E`) in IDT setup to symbolic constants (e.g. `GDT64_KERNEL_CS`, `IDT_ATTR_PRESENT_RING0_INT_GATE`).

---

## Phase 2: SerpentC Runtime & Interpreter (M1)

Replaces the initial print stub with a lightweight, graphics-enabled interpreter.

### 2.1 Lexer & AST Definition
* **Target:** `kernel/serpentc/lexer.c`, `kernel/serpentc/parser.c`, `kernel/serpentc/ast.h`
* **Goal:** Build a zero-allocation tokeniser and AST nodes for statement-based SerpentC code (variable assignments, primitive calls, integer expressions).

### 2.2 Built-in Primitive Binding & Execution Engine
* **Target:** `kernel/serpentc/interpreter.c`, `kernel/serpentc/builtins.c`
* **Goal:** Bind core runtime built-in functions:
  * `feather_draw_line(x1, y1, x2, y2, color)`
  * `feather_rect(x, y, w, h, color)`
  * `tonal_now()`
* Decouple graphics drivers from the interpreter core using function callback tables.

---

## Phase 3: Physical Memory Management & Diagnostics

Transition from static memory regions to dynamic bootloader-driven allocation.

### 3.1 Multiboot Physical Memory Allocator (M5)
* **Target:** `kernel/mm/phys.c`, `kernel/mm/phys.h`, `kernel/kernel.c`
* **Goal:** Parse `mmap_addr` and `mmap_length` from Multiboot structures to build a page-granular physical memory bitmap/allocator, replacing hardcoded heap ranges (`0x00400000`–`0x00FFFFFF`).

### 3.2 VGA Text-Mode Fallback (`0xB8000`) (M4)
* **Target:** `kernel/drivers/vga_text.c`, `kernel/drivers/vga_text.h`
* **Goal:** Add simple 80x25 text-mode console driver targeting `0xB8000` for early boot diagnostics and panic screens prior to VBE initialization.

### 3.3 Serial Port `kprintf` Logging (L3)
* **Target:** `kernel/arch/x86_64/serial.c`, `kernel/arch/x86_64/serial.h`
* **Goal:** Write UART COM1 (`0x3F8`) driver to pipe `kprintf` logs and panic stack traces to QEMU `-serial stdio`.

---

## Phase 4: Driver Extensions, Optimization & CI

### 4.1 Framebuffer Hot-Path Optimization (M2, M3)
* Benchmark and implement `rep stosq` / `rep movsq` primitives in `vbe.c`.
* Split unchecked `vbe_put_pixel` from checked `vbe_draw_pixel` for hoisted caller clipping loops.

### 4.2 PCI Bus Scanning (M7)
* Scan all PCI buses (0..255) and support 64-bit BAR decoding in `pci_find_vga_lfb`.

### 4.3 Automated CI & Smoke Test (L1)
* Add `.github/workflows/build.yml` to compile kernel and run headless QEMU boot test.

---

## Verification Plan

### Automated Verification
* `make clean && make` to ensure clean build without warnings.
* Launch kernel in QEMU (`./qemu.sh`) to verify stability, keyboard input, and rendering correctness.

### Manual Verification
* Test PS/2 key press/release rate handling under rapid input.
* Test Pyramid rendering at top-left corner coordinates (`cy = 0`, `cy = 10`) to confirm absence of wrapping/underflow.
* Inspect Multiboot boot logs to verify VBE probe skipping when pre-configured.
