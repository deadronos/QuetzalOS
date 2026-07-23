# QuetzalOS Phase 1 Implementation Plan

> **Branch:** `feature/improvements`  
> **Phase:** 1 (High-Priority Correctness & Safety)  
> **Status:** Draft / Pending Review  

This document details the specific technical changes for **Phase 1** of the QuetzalOS roadmap, addressing high-priority correctness issues, atomic state safety, geometry underflow bugs, VBE initialization safeguards, and segment selector cleanliness.

---

## Technical Specifications & Changes

### 1. Atomic PS/2 Keyboard Ring Buffer (H1)
* **Files:** `kernel/drivers/ps2_keyboard.c`
* **Problem:** `kbd_head` and `kbd_tail` (uint16_t) are mutated by IRQ1 (`keyboard_handler`) while read concurrently by `keyboard_has_char` and `keyboard_getchar` from the main execution loop. Compiler reordering or split non-atomic 16-bit instructions could lead to torn ring buffer indices.
* **Implementation Details:**
  * Include `<stdatomic.h>` in `ps2_keyboard.c`.
  * Convert indices to `static _Atomic uint16_t kbd_head = 0;` and `static _Atomic uint16_t kbd_tail = 0;`.
  * Use `atomic_load_explicit(&kbd_head, memory_order_relaxed)` and `atomic_load_explicit(&kbd_tail, memory_order_relaxed)` for lock-free, torn-free ring buffer queries.
  * Use `atomic_store_explicit(&kbd_head, next, memory_order_release)` in the interrupt handler and `atomic_store_explicit(&kbd_tail, next, memory_order_release)` in `keyboard_getchar`.

---

### 2. Geometry Underflow & Bounds Clipping (H2)
* **Files:** `kernel/graphics/ritual_geo.c`
* **Problem:** Rendering routines calculate vertical/horizontal origins using unsigned subtraction (e.g. `uint32_t y = cy - (i * step_height)`). When `cy` or `cx` are near the top/left edge of the screen, integer underflow wraps to `0xFFFFFFFF`, causing memory writes outside the framebuffer.
* **Implementation Details:**
  * In `draw_step_pyramid`:
    * Add horizontal origin underflow check: `uint32_t x = (cx > (w / 2)) ? (cx - (w / 2)) : 0;`
    * Add vertical step underflow check: Ensure `cy >= (i + 1) * step_height` before computing `y`.
    * Add temple top bounds check: Ensure `cy >= (steps * step_height) + top_h` before rendering the sanctuary rect.
  * In `draw_feathered_serpent`:
    * Guard vertical wave offset when `start_y` is less than `abs(wave)`.
  * In `draw_sun_disk`:
    * Guard `cx - radius` and `cy - radius` calculations against unsigned underflow.

---

### 3. Preserving Bootloader VBE Configuration (H4)
* **Files:** `kernel/drivers/vbe.c`
* **Problem:** `vbe_init` unconditionally issues Bochs VBE DISPI IO port commands (`bochs_vbe_set_mode`). When running on hardware or under Multiboot bootloaders that pre-configured a native graphics mode, this clobbers the active display mode.
* **Implementation Details:**
  * Add `static int externally_configured = 0;` flag in `vbe.c`.
  * In `vbe_init`: Set `externally_configured = 1` if `lfb_addr != 0` (supplied via Multiboot info).
  * Only invoke `bochs_vbe_set_mode((uint16_t)fb_w, (uint16_t)fb_h, 32)` when `externally_configured == 0` (i.e. when framebuffer mode was probed via PCI).

---

### 4. Named Segment Selector & IDT Attribute Constants (M6)
* **Files:** `kernel/arch/x86_64/segments.h` (NEW), `kernel/arch/x86_64/idt.c`
* **Problem:** Segment selectors (`0x08`) and IDT attribute flags (`0x8E`) are hardcoded as raw hex literals in `idt.c`.
* **Implementation Details:**
  * Create `kernel/arch/x86_64/segments.h` declaring:
    * `GDT64_KERNEL_CS` (`0x08u`)
    * `GDT64_KERNEL_DS` (`0x10u`)
    * `IDT_ATTR_PRESENT_RING0_INT_GATE` (`0x8Eu`)
  * Include `segments.h` in `idt.c` and replace raw hex values in `idt_set_gate` calls for timer (IRQ0) and keyboard (IRQ1) interrupt gates.

---

## File Modification Summary

| Target File | Status | Description |
|---|---|---|
| [kernel/drivers/ps2_keyboard.c](file:///Users/openclaw/Github/QuetzalOS/kernel/drivers/ps2_keyboard.c) | **MODIFY** | Use `_Atomic uint16_t` for `kbd_head`/`kbd_tail` ring buffer |
| [kernel/graphics/ritual_geo.c](file:///Users/openclaw/Github/QuetzalOS/kernel/graphics/ritual_geo.c) | **MODIFY** | Underflow guards and screen-edge clipping for ritual geometry |
| [kernel/drivers/vbe.c](file:///Users/openclaw/Github/QuetzalOS/kernel/drivers/vbe.c) | **MODIFY** | Skip `bochs_vbe_set_mode` when `lfb_addr` is supplied by Multiboot |
| [kernel/arch/x86_64/segments.h](file:///Users/openclaw/Github/QuetzalOS/kernel/arch/x86_64/segments.h) | **NEW** | GDT segment selectors and IDT gate attribute constants |
| [kernel/arch/x86_64/idt.c](file:///Users/openclaw/Github/QuetzalOS/kernel/arch/x86_64/idt.c) | **MODIFY** | Use symbolic segment constants from `segments.h` in `idt_init` |

---

## Verification Plan

### Automated Build Verification
* Run `make clean && make` to ensure all 4 components compile with zero warnings or errors.

### Runtime Verification
1. **PS/2 Keyboard Stress Test:** Launch kernel in QEMU (`./qemu.sh`) and type input rapidly to verify lock-free, atomic buffer enqueue/dequeue without dropped or corrupted keystrokes.
2. **Ritual Geometry Clipping:** Render pyramids with small `cy` coordinates (`cy = 5`, `cy = 10`) to confirm zero underflow / wrapping or screen boundary corruption.
3. **Multiboot VBE Protection:** Confirm kernel retains Multiboot-provided framebuffer resolution when `lfb_addr` is passed.
