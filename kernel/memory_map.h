// filepath: kernel/memory_map.h
#ifndef QUETZAL_MEMORY_MAP_H
#define QUETZAL_MEMORY_MAP_H

// =============================================================================
// QuetzalOS Physical Memory Map
//
// Single source of truth for physical addresses used by the kernel and
// runtime. These values MUST stay in sync with the linker script
// (linker.ld), the paging setup in boot.asm, and the documentation in
// docs/ARCHITECTURE.md.
//
// All addresses assume identity mapping (Virtual == Physical) under the
// flat 4 GB single address space layout.
// =============================================================================

#include <stdint.h>

/* Page Table Symbols from boot.asm */
extern uint8_t pml4_table[];
extern uint8_t pdpt_table[];
extern uint8_t pd_tables[];

// --- Boot / Kernel Image ----------------------------------------------------
#define QOS_KERNEL_BASE_PHYS        0x00100000u   // 1 MB - Kernel ELF image
#define QOS_KERNEL_SIZE_MAX         0x00100000u   // 1 MB max image size
#define QOS_KERNEL_STACK_TOP        0x00400000u   // Stack grows down from here
#define QOS_KERNEL_STACK_SIZE       0x00100000u   // 1 MB stack region

// --- System Tables (GDT64, IDT64, PML4/PDPT/PD) -----------------------------
#define QOS_PAGE_TABLE_BASE         ((uintptr_t)pml4_table)
#define QOS_PML4_ADDR               ((uintptr_t)pml4_table)
#define QOS_PDPT_ADDR               ((uintptr_t)pdpt_table)
#define QOS_PD_BASE                 ((uintptr_t)pd_tables)

// --- Kernel Dynamic Heap ----------------------------------------------------
#define QOS_KERNEL_HEAP_BASE        0x00400000u   // 12 MB heap
#define QOS_KERNEL_HEAP_END         0x01000000u

// --- SerpentC JIT Arena -----------------------------------------------------
#define QOS_SERPENTC_ARENA_BASE     0x01000000u   // 48 MB executable arena
#define QOS_SERPENTC_ARENA_END      0x04000000u

// --- Graphics --------------------------------------------------------------
#define QOS_BACKBUFFER_BASE         0x04000000u   // 32 MB double-buffer RAM
#define QOS_BACKBUFFER_END          0x06000000u
#define QOS_BACKBUFFER_BYTES        (QOS_BACKBUFFER_END - QOS_BACKBUFFER_BASE)
#define QOS_DEFAULT_LFB_PHYS        0xFD000000u   // QEMU/BOCHS VBE LFB
#define QOS_DEFAULT_FB_WIDTH        800u
#define QOS_DEFAULT_FB_HEIGHT       600u
#define QOS_DEFAULT_FB_PITCH        3200u         // bytes per scanline (800*4)

// --- Helpers ---------------------------------------------------------------
// Convert a #defined byte count to a quadword count for SIMD copies.
#define QOS_BYTES_TO_QUADS(b)       ((b) / 8u)

#endif /* QUETZAL_MEMORY_MAP_H */
