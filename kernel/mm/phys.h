#ifndef QOS_PHYS_MM_H
#define QOS_PHYS_MM_H

#include <stdint.h>
#include <stddef.h>
#include "../kernel.h"

#define PAGE_SIZE 4096u

typedef struct {
    uint32_t size;
    uint64_t addr;
    uint64_t len;
    uint32_t type;
} __attribute__((packed)) multiboot_mmap_entry_t;

void phys_init(multiboot_info_t* mb_info);
uintptr_t phys_alloc_page(void);
void phys_free_page(uintptr_t page_addr);
uint64_t phys_get_free_memory(void);
uint64_t phys_get_total_memory(void);

#endif /* QOS_PHYS_MM_H */
