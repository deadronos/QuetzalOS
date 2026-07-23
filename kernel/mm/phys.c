#include "phys.h"
#include "../memory_map.h"
#include "../arch/x86_64/serial.h"

#define MAX_PHYS_MEM 0x100000000ULL // 4 GB identity map cap
#define TOTAL_PAGES  (MAX_PHYS_MEM / PAGE_SIZE) // 1,048,576 pages
#define BITMAP_SIZE  (TOTAL_PAGES / 8u)         // 128 KB bitmap

static uint8_t page_bitmap[BITMAP_SIZE];
static uint64_t total_memory_bytes = 0;
static uint64_t free_memory_bytes = 0;

static void set_bit(size_t page_idx) {
    if (page_idx < TOTAL_PAGES) {
        page_bitmap[page_idx / 8u] |= (1u << (page_idx % 8u));
    }
}

static void clear_bit(size_t page_idx) {
    if (page_idx < TOTAL_PAGES) {
        page_bitmap[page_idx / 8u] &= ~(1u << (page_idx % 8u));
    }
}

static int test_bit(size_t page_idx) {
    if (page_idx >= TOTAL_PAGES) return 1;
    return (page_bitmap[page_idx / 8u] & (1u << (page_idx % 8u))) != 0;
}

static void mark_range_reserved(uint64_t start_addr, uint64_t size) {
    uint64_t start_page = start_addr / PAGE_SIZE;
    uint64_t end_page = (start_addr + size + PAGE_SIZE - 1) / PAGE_SIZE;

    for (uint64_t p = start_page; p < end_page && p < TOTAL_PAGES; p++) {
        if (!test_bit(p)) {
            set_bit(p);
            if (free_memory_bytes >= PAGE_SIZE) {
                free_memory_bytes -= PAGE_SIZE;
            }
        }
    }
}

void phys_init(multiboot_info_t* mb_info) {
    /* Initially mark all pages as reserved (1) */
    for (size_t i = 0; i < BITMAP_SIZE; i++) {
        page_bitmap[i] = 0xFF;
    }
    total_memory_bytes = 0;
    free_memory_bytes = 0;

    /* Parse Multiboot memory map if available (bit 6 set) */
    if (mb_info && (mb_info->flags & (1u << 6)) && mb_info->mmap_addr && mb_info->mmap_length > 0) {
        uintptr_t mmap_curr = mb_info->mmap_addr;
        uintptr_t mmap_end  = mb_info->mmap_addr + mb_info->mmap_length;

        while (mmap_curr < mmap_end) {
            multiboot_mmap_entry_t* entry = (multiboot_mmap_entry_t*)mmap_curr;

            if (entry->type == 1 && entry->addr < MAX_PHYS_MEM) {
                uint64_t start_page = entry->addr / PAGE_SIZE;
                uint64_t count = entry->len / PAGE_SIZE;
                total_memory_bytes += entry->len;

                for (uint64_t p = 0; p < count; p++) {
                    uint64_t page_idx = start_page + p;
                    if (page_idx < TOTAL_PAGES) {
                        clear_bit(page_idx);
                        free_memory_bytes += PAGE_SIZE;
                    }
                }
            }

            mmap_curr += entry->size + sizeof(uint32_t);
        }
    } else {
        /* Fallback if mmap is absent: assume 64 MB usable RAM */
        total_memory_bytes = 64 * 1024 * 1024ULL;
        for (uint64_t p = 0; p < (total_memory_bytes / PAGE_SIZE); p++) {
            clear_bit(p);
            free_memory_bytes += PAGE_SIZE;
        }
    }

    /* Mark reserved low 1MB region (BIOS, IVT, real mode tables) */
    mark_range_reserved(0x00000000, 0x00100000);

    /* Mark Kernel image region (1MB .. 4MB) */
    mark_range_reserved(QOS_KERNEL_BASE_PHYS, 0x00300000);

    /* Mark SerpentC Arena (16MB .. 64MB) */
    mark_range_reserved(QOS_SERPENTC_ARENA_BASE, QOS_SERPENTC_ARENA_END - QOS_SERPENTC_ARENA_BASE);

    /* Mark Backbuffer region (64MB .. 96MB) */
    mark_range_reserved(QOS_BACKBUFFER_BASE, QOS_BACKBUFFER_BYTES);

    kprintf("[PHYS] Allocator Initialized: Total %d MB, Free %d MB\n",
            (int)(total_memory_bytes / (1024 * 1024)),
            (int)(free_memory_bytes / (1024 * 1024)));
}

uintptr_t phys_alloc_page(void) {
    for (size_t p = 0; p < TOTAL_PAGES; p++) {
        if (!test_bit(p)) {
            set_bit(p);
            if (free_memory_bytes >= PAGE_SIZE) {
                free_memory_bytes -= PAGE_SIZE;
            }
            return (uintptr_t)(p * PAGE_SIZE);
        }
    }
    return 0; // Out of physical memory
}

void phys_free_page(uintptr_t page_addr) {
    size_t page_idx = page_addr / PAGE_SIZE;
    if (page_idx < TOTAL_PAGES && test_bit(page_idx)) {
        clear_bit(page_idx);
        free_memory_bytes += PAGE_SIZE;
    }
}

uint64_t phys_get_free_memory(void) {
    return free_memory_bytes;
}

uint64_t phys_get_total_memory(void) {
    return total_memory_bytes;
}
