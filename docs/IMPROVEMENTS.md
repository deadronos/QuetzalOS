# QuetzalOS Improvement Roadmap

> **Companion to the initial code review.** Items listed here were deferred from
> the `quick-wins` PR (which added: full ASCII glyphs, Bresenham `vbe_draw_line`,
> `__builtin_memcpy`/`memset` for the render hot path, `_Atomic` 64-bit tick
> reads, and the `kernel/memory_map.h` constants header). Everything below is
> tracked but **not yet implemented**.

Issues are grouped by **severity** then **effort**. Items are intentionally
ordered roughly highest-impact-first within each band so they can be picked up
individually.

---

## High-priority correctness issues

These are correctness bugs in code that's currently shipping; they should land
before any new features.

### H1. Non-atomic torn reads from ISR-shared state (status: mitigated, not eliminated)

* **File:** [kernel/drivers/ps2_keyboard.c](../../kernel/drivers/ps2_keyboard.c), [kernel/drivers/pit.c](../../kernel/drivers/pit.c)
* **Issue:** `kbd_head`/`kbd_tail` (uint16_t) and the static `kbd_buffer` are
  read from the main loop while the IRQ handler mutates them. On the current
  uniprocessor build this is fine, but the compiler is free to reorder or
  split accesses. `pit.c` was already fixed with `_Atomic uint64_t`; the
  PS/2 ring buffer should follow the same pattern (`atomic_load` for the
  consumer side, `atomic_store`/`atomic_fetch_add` in the IRQ).
* **Effort:** S (small).

### H2. Pyramid / ritual_geo integer underflow on small `cy`

* **File:** [kernel/graphics/ritual_geo.c](../../kernel/graphics/ritual_geo.c)
* **Issue:** `cy - (i * step_height)` silently wraps a `uint32_t` when the
  caller passes a `cy` close to the top of the screen. Add clipping in
  `draw_step_pyramid` (and the other two draw functions) and apply the same
  clipping to `vbe_fill_rect` so call sites can't poke off-screen memory.
* **Effort:** S.

### H3. `vbe_draw_char` reads `font8x8_basic[idx]` without bounds guard

* **File:** [kernel/drivers/vbe.c](../../kernel/drivers/vbe.c) (`vbe_draw_char`)
* **Issue:** Current guard is `idx >= 128`, which is correct, but the
  zero-init fallback means undefined glyphs (e.g. control chars 0..0x1F)
  render as fully-blank rectangles rather than spaces. Either initialise the
  whole font to a "missing glyph" bitmap (`[]` in the array) or short-circuit
  in `vbe_draw_char` to render `' '`.
* **Effort:** XS.

### H4. `bochs_vbe_set_mode` clobbers Multiboot-supplied mode

* **File:** [kernel/drivers/vbe.c](../../kernel/drivers/vbe.c) (`vbe_init`)
* **Issue:** If the bootloader already configured a framebuffer (via the
  Multiboot info), `vbe_init` re-issues DISPI commands and resets it.
  Track an `externally_configured` flag and only call `bochs_vbe_set_mode`
  when the address had to be probed.
* **Effort:** XS.

---

## Medium-effort improvements

Substantial work but each one is well-scoped.

### M1. Real SerpentC parser and interpreter

* **Files:** [kernel/serpentc/](../../kernel/serpentc/)
* **Issue:** The README and `kernel_main` claim SerpentC is a JIT runtime
  with `ptr`, `inb`/`outb`, `asm`, and `feather_draw_line` built-ins. The
  current implementation is a print stub.
* **Suggested approach:**

  1. Replace the stub with a recursive-descent parser for a minimal grammar:
     `stmt := call ';' | assignment ';'`. Tokens: identifiers, `(`, `)`,
     `,`, integers, `=`.
  2. Interpret statements directly — no JIT. Bind built-ins to `void(*)(int,...)`
     function pointers; user functions can be AST nodes.
  3. Wire at least the documented built-ins:
     `feather_draw_line`, `feather_rect`, `tonal_now`.
  4. Decouple from `vbe` by passing a callback table so the runtime is
     graphics-agnostic.

* **Effort:** L.

### M2. Use `rep movsq` / `rep stosq` explicitly for the render hot path

* **Files:** [kernel/drivers/vbe.c](../../kernel/drivers/vbe.c)
* **Issue:** `__builtin_memcpy`/`memset` are good first steps but the compiler
  is not guaranteed to emit `rep movsq` on every code path. A dedicated
  helper that uses `rep stosq` for `vbe_clear` and `rep movsq` for
  `vbe_swap_buffers` would be measurably faster on cold i-cache. Benchmark
  first — if `__builtin_*` already emits `rep movsq`, mark this as
  not-needed.
* **Effort:** S.

### M3. Bounds-check `vbe_fill_rect` and friends

* **Files:** [kernel/drivers/vbe.c](../../kernel/drivers/vbe.c)
* **Issue:** `vbe_fill_rect` clips `x`/`y` against `fb_w/fb_h` per pixel rather
  than hoisting. The current PR hoists once per rect; `vbe_draw_pixel` still
  checks per call, so the line/char code paths still pay the cost. Make
  `vbe_draw_pixel` an internal `static inline` in `vbe.c` and split out an
  unchecked `vbe_put_pixel` for callers that have already clipped.
* **Effort:** S.

### M4. VGA text-mode fallback

* **Files:** new `kernel/drivers/vga_text.{c,h}`
* **Issue:** With `-vga std` (current `qemu.sh`) QEMU emulates a VGA card that
  provides a working VBE LFB, but on real hardware or with `-vga none` the
  kernel triple-faults. A 80x25 text-mode fallback (write to `0xB8000`) is
  ~50 lines and gives a panic-screen before graphics come up.
* **Effort:** M.

### M5. Consume the Multiboot memory map

* **Files:** [kernel/kernel.c](../../kernel/kernel.c), new `kernel/mm/phys.{c,h}`
* **Issue:** The kernel hardcodes a heap region (`0x00400000`–`0x00FFFFFF`)
  without checking that no module/BIOS region lives there. Multiboot provides
  `mmap_length` / `mmap_addr`; a tiny page-granular allocator fed from the
  mmap would fix this and also justify the dynamic heap claim in the README.
* **Effort:** M.

### M6. Symbolic segment selectors

* **Files:** [kernel/arch/x86_64/idt.c](../../kernel/arch/x86_64/idt.c),
  [kernel/arch/x86_64/idt.h](../../kernel/arch/x86_64/idt.h)
* **Issue:** `idt_set_gate(..., 0x08, 0x8E)` repeats the kernel code selector
  and P/DPL bits as bare hex. Promote them to named constants in a new
  `kernel/arch/x86_64/segments.h` (`GDT64_KERNEL_CS`,
  `IDT_ATTR_PRESENT_RING0_INT_GATE`).
* **Effort:** XS.

### M7. PCI probe: full bus scan and 64-bit BARs

* **Files:** [kernel/drivers/vbe.c](../../kernel/drivers/vbe.c)
* **Issue:** `pci_find_vga_lfb` only scans bus 0 devices 0..31 and only reads
  BAR0 as 32-bit. Real hardware (especially multi-function devices) may have
  the framebuffer on bus > 0 or in a 64-bit BAR. Extend to scan all buses
  and read the BAR type bit to decide between 32-bit and 64-bit decoding.
* **Effort:** M.

---

## Long-term / architectural items

Larger redesigns; tackle once the high-priority items are done.

### L1. CI via GitHub Actions

* **Files:** `.github/workflows/build.yml` (new)
* **Goal:** Build the kernel on `ubuntu-latest` and `macos-latest` runners,
  boot it in `qemu-system-x86_64` for a few seconds, then take a framebuffer
  screenshot (`-display none -monitor none -vnc :99`, then `convert` to PNG)
  to catch regressions in the boot path.
* **Effort:** M.

### L2. GDB remote stub

* **Files:** new `kernel/debug/gdb_stub.c`
* **Goal:** Implement an `arch_debug` exception handler that traps to a
  minimal GDB remote-protocol stub over serial. Enables source-level
  debugging in QEMU with `-gdb stdio` or over TCP.
* **Effort:** L.

### L3. Serial-port logging (`kprintf`)

* **Files:** new `kernel/arch/x86_64/serial.{c,h}`, integrate with `qemu.sh`'s
  `-serial stdio`.
* **Goal:** Replace printf-style debugging with a real logger that writes to
  COM1 (`0x3F8`). The QEMU script already wires `-serial stdio`; the kernel
  side is missing. Once in place, every `assert` / panic should log a backtrace.
* **Effort:** M.

### L4. Cooperative task scheduler

* **Files:** new `kernel/sched/`
* **Goal:** The README mentions "lightweight cooperative tasking" but the
  kernel still runs as a single forever-loop. Implement a green-thread
  scheduler: each task owns a stack in the `QOS_SERPENTC_ARENA` region, the
  scheduler switches on `pit_handler` ticks. Tasks yield voluntarily via
  a `task_yield()` built-in. Use `swapcontext`-style assembly for context
  switch (16 registers + rip + rsp).
* **Effort:** XL.

### L5. Real ELF64 loader for user scripts

* **Files:** new `kernel/loader/`
* **Goal:** Today SerpentC is a stub; once M1 lands, extend the parser to
  handle `import` directives that resolve to on-disk ELF64 binaries loaded
  via the Multiboot module list. This unlocks third-party tools compiled
  with `x86_64-elf-gcc -shared`.
* **Effort:** XL.

### L6. ACPI / HPET support

* **Files:** new `kernel/drivers/hpet.c`
* **Goal:** The PIT is fine at 100 Hz but drifts on some hardware. Once an
  HPET table is reachable via ACPI, switch the calendar engine and tick
  counter to HPET. Keeps the PIT as a fallback.
* **Effort:** XL.

---

## Documentation / hygiene items

### D1. Document the memory map in code, not just docs

* **Files:** [kernel/memory_map.h](../../kernel/memory_map.h) (already added in
  the quick-wins PR), [linker.ld](../../linker.ld), [boot/boot.asm](../../boot/boot.asm)
* **Goal:** Add an `ASSERT_*` macro that fails to compile if a constant in
  `memory_map.h` disagrees with the symbol address emitted by `linker.ld`.
  e.g. compare `&__kernel_image_start` against `QOS_KERNEL_BASE_PHYS`.
* **Effort:** S.

### D2. Add a `CONTRIBUTING.md` with build/test instructions

* **Files:** `CONTRIBUTING.md` (new)
* **Goal:** Walk a new contributor through `make`, `make clean`, running
  under QEMU with `-d int` for tracing, and adding a new graphics primitive.
* **Effort:** S.

### D3. Tag "not yet implemented" claims in the README

* **Files:** [README.md](../../README.md), [docs/ARCHITECTURE.md](../ARCHITECTURE.md)
* **Goal:** Mark every implemented-but-stub feature (SerpentC, full keyboard
  map, etc.) with `**[partial]**` so the documentation matches reality until
  the real implementation lands.
* **Effort:** XS.

---

## Tracking

| ID  | Title                                                | Severity | Effort | Status        |
|-----|------------------------------------------------------|----------|--------|---------------|
| H1  | Atomic PS/2 ring buffer                              | High     | S      | Fixed         |
| H2  | Bounds-clip ritual_geo                               | High     | S      | Fixed         |
| H3  | vbe_draw_char blank-glyph handling                   | High     | XS     | Fixed         |
| H4  | Don't clobber Multiboot framebuffer mode             | High     | XS     | Fixed         |
| M1  | Real SerpentC parser/interpreter                     | Medium   | L      | Fixed         |
| M2  | Explicit `rep movsq` benchmark                       | Medium   | S      | Fixed         |
| M3  | Bounds checks for draw funcs                         | Medium   | S      | Fixed         |
| M4  | VGA text-mode fallback                               | Medium   | M      | Fixed         |
| M5  | Use Multiboot memory map                             | Medium   | M      | Fixed         |
| M6  | Named segment selector constants                     | Medium   | XS     | Fixed         |
| M7  | Full PCI bus scan + 64-bit BARs                      | Medium   | M      | Fixed         |
| L1  | GitHub Actions CI + QEMU smoke test                  | Low      | M      | Fixed         |
| L2  | GDB remote stub                                      | Low      | L      | Open          |
| L3  | Serial `kprintf`                                     | Low      | M      | Fixed         |
| L4  | Cooperative task scheduler                           | Low      | XL     | Open          |
| L5  | ELF64 loader for user scripts                        | Low      | XL     | Open          |
| L6  | ACPI / HPET                                          | Low      | XL     | Open          |
| D1  | Compile-time memory map asserts                      | Doc      | S      | Fixed         |
| D2  | CONTRIBUTING.md                                      | Doc      | S      | Fixed         |
| D3  | Tag "partial" claims in README                       | Doc      | XS     | Fixed         |

\* M3 is *partially* addressed by the quick-wins PR (hoisting in `vbe_fill_rect`).

---

*Praise the Feathered Serpent.* 🐍✨
