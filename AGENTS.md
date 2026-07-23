# QuetzalOS Agent Guidelines & Development Standards

> **Scope:** Guidelines for AI agents (and human contributors) implementing features, drivers, memory management, or refactoring in the **QuetzalOS** repository.

---

## 🏛️ System Architecture Context

QuetzalOS (TonalOS) is a **monoprogrammatic, single-privilege 64-bit flat-binary environment** running in **Ring 0**.

* **Memory Model:** Flat 4 GB identity-mapped physical/virtual address space. No Ring 3 userland boundary; all code executes with full privileges.
* **Concurrency Model:** Uniprocessor IRQ interrupts (`PIT` timer at 100 Hz, `PS/2` keyboard at IRQ1).
* **Freestanding C:** Built with `-ffreestanding -mcmodel=kernel -mno-red-zone -mno-mmx -mno-sse -mno-sse2 -nostdlib -fno-builtin`. No standard C library routines (`memcpy`, `printf`, `malloc`) unless explicitly implemented in-tree.

---

## 🔄 Mandatory Agent Workflow

Every feature implementation or bug fix task **MUST** follow this 4-step workflow:

```
┌─────────────────┐     ┌──────────────────┐     ┌────────────────────────────┐     ┌───────────────────┐
│ 1. Plan & Scope │ ──> │ 2. Implementation│ ──> │ 3. Adversarial Self-Review │ ──> │ 4. Remediation    │
└─────────────────┘     └──────────────────┘     └────────────────────────────┘     └───────────────────┘
```

### Step 1: Plan & Scope
* Inspect relevant source files before making changes.
* Lay out phased implementation goals in plan artifacts / documentation.

### Step 2: Implementation
* Write modular, well-commented freestanding C99 / Assembly code.
* Ensure code compiles cleanly with `-Wall -Wextra`.

### Step 3: Mandatory Adversarial Self-Review
Before declaring any task complete, the agent **MUST** perform an explicit **Adversarial Code Review** of all new or modified code. Analyze the changes with zero assumptions, intentionally seeking:
1. Corner cases and boundary coordinate underflows.
2. Unsigned/signed integer cast overflows.
3. Concurrency / race conditions across ISR interrupt boundaries.
4. Hardware driver assumptions (e.g. Multiboot flags, video modes).
5. Desynchronization between assembly symbols, headers, and linker scripts.

### Step 4: Remediation & Verification
* Remediate all vulnerabilities or edge-case flaws identified during the adversarial review.
* Run `make clean && make` to verify zero compilation errors or warnings.
* Update tracking tables in [docs/IMPROVEMENTS.md](file:///Users/openclaw/Github/QuetzalOS/docs/IMPROVEMENTS.md) when roadmap items are fixed.

---

## 🛡️ Adversarial Audit Checklist

When conducting an adversarial code review, audit against these specific vulnerability classes:

### 1. Concurrency & ISR Safety
- [ ] Are variables shared between interrupt handlers (IRQs) and main loops declared `_Atomic` with explicit atomic operations (`stdatomic.h`)?
- [ ] Is memory ordering appropriate (`memory_order_relaxed`, `memory_order_acquire`, `memory_order_release`)?
- [ ] Are lock-free ring buffers protected against Multi-Consumer or Multi-Producer race conditions?
- [ ] Is ISR interrupt state (e.g., PIC EOI `outb(0x20, 0x20)`) handled on every early-return execution path?

### 2. Arithmetic & Bounds Safety
- [ ] Are unsigned coordinate subtractions (e.g. `cy - (i * height)`) guarded against underflow to `0xFFFFFFFF`?
- [ ] Are input coordinates converted to signed integer types (e.g. in Bresenham algorithms) clamped to prevent `INT_MAX` wrap and infinite loops?
- [ ] Do text and line rendering routines wrap or break cleanly at framebuffer boundaries (`fb_w`, `fb_h`)?

### 3. Hardware & Bootloader Compatibility
- [ ] Does Multiboot info validation check specific mode types (`framebuffer_type == 1` Direct RGB, `bpp == 32`) rather than assuming defaults?
- [ ] Are physical memory allocations bounded within the 4 GB identity-mapped page table limit?
- [ ] Are hardware IO ports (`inb`/`outb`/`inw`/`outw`/`inl`/`outl`) accessed with appropriate timing delays (`io_wait()`) where necessary?

### 4. Memory Layout & Linker Integrity
- [ ] Are page table addresses in `memory_map.h` mapped directly to assembly symbols (`&pml4_table`, `&pdpt_table`, `&pd_tables`) rather than hardcoded magic offsets?
- [ ] Is the kernel stack allocated before critical page structures in `.bss` to prevent stack overflow corruption?

---

## 📜 Code Style & Build Commands

* **Build Command:** `make clean && make`
* **Test Command:** `./qemu.sh`
* **C Standard:** C99 (`-std=c99` / freestanding GCC)
* **Headers:** Prefer explicit freestanding headers (`<stdint.h>`, `<stdatomic.h>`, `<stddef.h>`).
