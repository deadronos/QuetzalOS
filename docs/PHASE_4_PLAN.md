# QuetzalOS Phase 4 Implementation Plan

> **Branch:** `feature/improvements`  
> **Phase:** 4 (Optimization, Driver Upgrades, CI & Documentation)  
> **Status:** Draft / Pending Review  

This document details the technical plan for **Phase 4**, completing graphics hot-path optimizations, full 256-bus PCI scanning, 64-bit BAR decoding, GitHub Actions CI workflow, and documentation hygiene.

---

## Technical Specifications & Planned Changes

### 1. Framebuffer Hot-Path & Unchecked Pixel Plotting (M2, M3)
* **Target:** `kernel/drivers/vbe.c`, `kernel/drivers/vbe.h`
* **Changes:**
  * Define `static inline void vbe_put_pixel(uint32_t x, uint32_t y, uint32_t color)` for direct backbuffer writes without per-pixel bounds checking.
  * Update `vbe_fill_rect` to use `vbe_put_pixel` across pre-clipped scanline ranges.
  * Verify `rep stosq` (in `vbe_clear`) and `rep movsq` (in `vbe_swap_buffers`).

---

### 2. Full 256-Bus PCI Probe & 64-bit BAR Decoding (M7)
* **Target:** `kernel/drivers/vbe.c`
* **Changes:**
  * Expand `pci_find_vga_lfb` to iterate through all 256 buses, 32 devices, and multi-function devices (functions 0..7).
  * Read BAR type bits (`bar0 & 0x6`). If 64-bit BAR (`0x04`), read upper 32 bits from BAR1 (`0x14`).
  * Ensure decoded 64-bit LFB address falls within the identity-mapped 4 GB memory limit.

---

### 3. GitHub Actions CI Workflow (L1)
* **Target:** `.github/workflows/build.yml` (NEW)
* **Changes:**
  * Add GitHub Actions workflow on `ubuntu-latest`.
  * Install `nasm` and `gcc-x86-64-elf` / cross-compiler.
  * Run `make clean && make` to automate pull request build checks.

---

### 4. Memory Map Asserts & Documentation Hygiene (D1, D2, D3)
* **Target:** `kernel/kernel.c`, `CONTRIBUTING.md` (NEW), `README.md`, `docs/ARCHITECTURE.md`
* **Changes:**
  * Add kernel startup memory map assertion checks in `kernel_main`.
  * Create `CONTRIBUTING.md` with build, test, and adversarial code review rules.
  * Update `README.md` and `docs/ARCHITECTURE.md` to accurately reflect implemented features.

---

## File Addition & Modification Summary

| File Path | Status | Description |
|---|---|---|
| [kernel/drivers/vbe.c](file:///Users/openclaw/Github/QuetzalOS/kernel/drivers/vbe.c) | **MODIFY** | `vbe_put_pixel`, full 256-bus PCI scan & 64-bit BAR decoding |
| [.github/workflows/build.yml](file:///Users/openclaw/Github/QuetzalOS/.github/workflows/build.yml) | **NEW** | GitHub Actions CI build workflow |
| [CONTRIBUTING.md](file:///Users/openclaw/Github/QuetzalOS/CONTRIBUTING.md) | **NEW** | Build/test guidelines and contributor standards |
| [README.md](file:///Users/openclaw/Github/QuetzalOS/README.md) | **MODIFY** | Update implementation status and feature tags |
| [docs/ARCHITECTURE.md](file:///Users/openclaw/Github/QuetzalOS/docs/ARCHITECTURE.md) | **MODIFY** | Update system architecture documentation |

---

## Verification Plan

### Automated Build Verification
* Run `make clean && make` to verify compilation with zero warnings across all modules.

### Runtime Verification
* Execute `./qemu.sh` in QEMU.
* Inspect PCI scan logs via COM1 serial output to confirm full bus scanning and LFB decoding.
