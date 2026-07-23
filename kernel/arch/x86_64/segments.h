#ifndef QOS_SEGMENTS_H
#define QOS_SEGMENTS_H

#include <stdint.h>

/* GDT Segment Selectors */
#define GDT64_KERNEL_CS 0x08u
#define GDT64_KERNEL_DS 0x10u

/* IDT Gate Attributes */
#define IDT_ATTR_PRESENT_RING0_INT_GATE 0x8Eu

#endif /* QOS_SEGMENTS_H */
