# QuetzalOS (TonalOS) — Architectural Specification & Engineering Blueprint

## 1. System Overview & Core Philosophy

QuetzalOS (TonalOS) is a 64-bit bare-metal operating system architected as a single-privilege-level, single-address-space digital temple inspired by retro workstation environments (TempleOS, NeXTSTEP, SGI IRIX) and ancient Mesoamerican ritual cosmology dedicated to Quetzalcoatl.

```
+-----------------------------------------------------------------------------------+
|                        SERPENT-C RUNTIME / RITUAL APPLICATIONS                     |
+-----------------------------------------------------------------------------------+
|                           QUETZAL-OS KERNEL SUBSYSTEMS                            |
|    [Chronos / Calendar]    [VBE Framebuffer]    [PS/2 Input]    [Ritual Vector]   |
+-----------------------------------------------------------------------------------+
|                      FLAT 64-BIT IDENTITY-MAPPED ADDRESS SPACE                     |
|                           RING 0 / CPL=0 ONLY EXECUTION                           |
+-----------------------------------------------------------------------------------+
|                                 HARDWARE / QEMU x86_64                            |
+-----------------------------------------------------------------------------------+
```

### Key Architectural Principles:
1. **Single Privilege Level (Ring 0 / CPL=0):**
   - The OS operates exclusively in Ring 0. There is no separation between userland and kernel space.
   - Eliminates context-switching overhead, ring transition latency (`syscall`/`sysret` or `int 0x80`), and complex descriptor privilege management.
   - Applications, drivers, and the kernel share full hardware privileges.

2. **Single Address Space (SAS):**
   - Flat identity mapping: Virtual Address == Physical Address across the entire address space (`0x0000000000000000` to physical memory limits).
   - No virtual memory isolation or page table swapping (`CR3` reloading) between tasks.
   - Lightweight cooperative tasking / green threads operating within a unified heap.

3. **Sacred Time & Visual Identity:**
   - Hardware interrupt clock (PIT/HPET) drives a dual calendar engine based on the *Tonalpohualli* (260-day ritual cycle) and *Xiuhpohualli* (365-day solar calendar).
   - Visual interface uses direct VBE linear framebuffers (800x600 @ 32-bit RGB) rendered using a custom 8x8 bitmap font engine and procedural Aztec vector geometry (step pyramids, solar disks, feathered serpents).

---

## 2. Bootloader & Memory Map Specifications

### 2.1 CPU State Transition Trajectory

The system boots via a Multiboot1-compliant header embedded in the kernel image, supported directly by GRUB or QEMU's direct kernel loader (`-kernel`).

```
[16-Bit Real Mode]  --->  [32-Bit Protected Mode]  --->  [64-Bit Long Mode]
 (BIOS Initialization)       (PE=1, GDT32 Loaded)        (PAE=1, LME=1, PG=1)
                                                         Identity Paging Active
```

1. **Phase 1: 32-Bit Protected Mode Entry (Multiboot Compliant)**
   - CPU enters `boot.asm` at 32-bit Protected Mode (`EAX = 0x2BADB002`, `EBX = Multiboot Info Struct`).
   - A20 line is guaranteed enabled by bootloader. Interrupts disabled (`cli`).

2. **Phase 2: Identity Paging Setup (PML4 -> PDPT -> PD)**
   - Allocates initial 4-level paging structures statically at `0x00200000`:
     - `PML4` (Page Map Level 4) at `0x20000`
     - `PDPT` (Page Directory Pointer Table) at `0x21000`
     - `PD` (Page Directory) at `0x22000`
   - Identity maps the first 1 GB of physical memory using 2 MB huge pages (`Page Directory Entries` with `PS` bit set to 1).

3. **Phase 3: Long Mode Enablement**
   - Set Physical Address Extension bit in `CR4` (`CR4.PAE = 1`).
   - Load PML4 address into `CR3`.
   - Enable Long Mode in Model Specific Register (MSR `0xC0000080` / `IA32_EFER`), setting bit 8 (`LME = 1`).
   - Enable Paging in `CR0` (`CR0.PG = 1` and `CR0.PE = 1`). CPU enters Compatibility Mode.

4. **Phase 4: Far Jump to 64-Bit Long Mode Entry**
   - Execute far jump `jmp 0x08:long_mode_start` to load `CS` with 64-bit Code Segment selector.
   - Reload segment registers (`DS`, `ES`, `FS`, `GS`, `SS`) with 64-bit Data Segment selector (`0x10`).
   - Set 64-bit stack pointer (`RSP`) to `0x00400000` (4MB mark, growing down).
   - Call `kernel_main` in `kernel.c`.

---

### 2.2 Physical Memory Layout Map

| Address Range | Size | Component / Purpose |
|---|---|---|
| `0x0000_0000 - 0x0000_03FF` | 1 KB | Real Mode Interrupt Vector Table (IVT) - Discarded post-boot |
| `0x0000_0400 - 0x0000_04FF` | 256 B | BIOS Data Area (BDA) |
| `0x0000_7C00 - 0x0000_7DFF` | 512 B | Stage 1 MBR Boot Sector (Legacy fallback) |
| `0x0000_8000 - 0x0009_FFFF` | ~600 KB | Low RAM / Multiboot Metadata Buffer |
| `0x000A_0000 - 0x000F_FFFF` | 384 KB | Video ROM / Legacy VGA Framebuffer / System BIOS |
| `0x0010_0000 - 0x001F_FFFF` | 1 MB | **Kernel ELF Image** (.text, .rodata, .data, .bss) |
| `0x0020_0000 - 0x0022_FFFF` | 192 KB | **Core System Tables** (GDT64, IDT64, Page Tables: PML4/PDPT/PD) |
| `0x0030_0000 - 0x003F_FFFF` | 1 MB | **Kernel Ring 0 Stack** (Top at `0x00400000`) |
| `0x0040_0000 - 0x00FF_FFFF` | 12 MB | **Kernel Dynamic Memory Heap** (Page Allocator / Malloc) |
| `0x0100_0000 - 0x03FF_FFFF` | 48 MB | **SerpentC JIT Arena & AST Pool** (Executable/Writable Ring 0 Memory) |
| `0x0400_0000 - 0x05FF_FFFF` | 32 MB | **Offscreen Double Buffer Framebuffer RAM** (800x600x32bpp) |
| `0xFD00_0000 - 0xFDFFFFFF` | ~16 MB | **VESA Linear Framebuffer (LFB)** MMIO Hardware Region |

---

## 3. Kernel Subsystems Architecture

### 3.1 Video & Graphics Subsystem (`vbe.c`, `ritual_geo.c`)
- **Framebuffer Driver:** Operates VESA Bios Extension (VBE) linear framebuffer (default 800x600 @ 32-bit BGR/RGB).
- **Double-Buffering:**
  - Backbuffer stored at physical RAM address `0x04000000`.
  - Frame renders to Backbuffer first; `vbe_swap()` blits backbuffer to LFB at `0xFD000000` via 64-bit SIMD/movsq memory copy.
- **Thematic Color Palette:**
  - `OBSIDIAN` (#0B0B0E / `0x000B0B0E`) - Background / Ritual void
  - `JADE` (#00A86B / `0x0000A86B`) - Primary glyph & serpent green
  - `AZTEC_GOLD` (#C59B27 / `0x00C59B27`) - Solar accents & borders
  - `TURQUOISE` (#40E0D0 / `0x0040E0D0`) - Sky/water elements & highlights
  - `COCHINEAL_RED` (#9E0027 / `0x009E0027`) - Sacred warnings & solar flare
- **Font & Procedural Geometry Renderer:**
  - Embedded 8x8 VGA bitmap font array rendering clean, retro-style typography.
  - Procedural Aztec ritual geometry engine: draws solar circles, step pyramids, and animated feathered serpents.

### 3.2 Chronos Subsystem (`pit.c`)
- **Hardware Timer:** Configures 8254 Programmable Interval Timer (PIT) at 100 Hz (10 ms per tick).
- **Tonalpohualli Calendar Engine (260-Day Sacred Almanac):**
  - Consists of 13 Trecena numbers (1 to 13) paired with 20 Day Signs (Veintenas):
    1. *Cipactli* (Crocodile), 2. *Ehecatl* (Wind), 3. *Calli* (House), 4. *Cuetzpalin* (Lizard), 5. *Coatl* (Serpent), 
    6. *Miquiztli* (Death), 7. *Mazatl* (Deer), 8. *Tochtli* (Rabbit), 9. *Atl* (Water), 10. *Itzcuintli* (Dog), 
    11. *Ozomatli* (Monkey), 12. *Malinalli* (Grass), 13. *Acatl* (Reed), 14. *Ocelotl* (Jaguar), 15. *Cuauhtli* (Eagle), 
    16. *Cozcacuauhtli* (Vulture), 17. *Ollin* (Movement), 18. *Tecpatl* (Flint), 19. *Quiahuitl* (Rain), 20. *Xochitl* (Flower)
- **Xiuhpohualli Calendar Engine (365-Day Solar Cycle):**
  - 18 months of 20 days each + 5 *Nemontemi* (sacred nameless days).

### 3.3 Input Subsystem (`ps2_keyboard.c`)
- PS/2 Keyboard IRQ 1 (Interrupt Vector 33).
- **ISR Architecture:**
  - Assembly stub (`irq1_stub`) saves 64-bit registers.
  - Calls `keyboard_handler()` in C.
  - Reads scan code from I/O port `0x60`.
  - Converts Set 1 scan codes to ASCII characters.
  - Pushes key events into a 256-byte circular ring buffer.
  - Sends End-of-Interrupt (`EOI = 0x20`) to PIC Master (`port 0x20`).

### 3.4 SerpentC Engine (`serpentc/`)
- A lightweight C-like JIT / Interpreter embedded directly in kernel space.
- Features direct raw pointers (`ptr`), raw port I/O (`inb`/`outb`), inline assembly support (`asm`), and built-in keywords.
