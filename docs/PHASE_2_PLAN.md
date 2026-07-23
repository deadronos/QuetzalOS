# QuetzalOS Phase 2 Implementation Plan

> **Branch:** `feature/improvements`  
> **Phase:** 2 (Real SerpentC Parser & Interpreter)  
> **Status:** Draft / Pending Review  

This document details the architecture and technical plan for **Phase 2**, implementing a real, freestanding lexer, statement parser, and execution engine for **SerpentC** in QuetzalOS.

---

## Technical Specifications & Architecture

### 1. Zero-Allocation Lexer & Tokenizer
* **Target:** `kernel/serpentc/lexer.h`, `kernel/serpentc/lexer.c`
* **Token Types:**
  * `TOKEN_IDENTIFIER` (e.g. `feather_draw_line`, `x`, `COLOR_GOLD`)
  * `TOKEN_INT_LITERAL` (e.g. `100`, `0x00FFFF`)
  * `TOKEN_LPAREN` (`(`), `TOKEN_RPAREN` (`)`)
  * `TOKEN_COMMA` (`,`)
  * `TOKEN_ASSIGN` (`=`)
  * `TOKEN_SEMICOLON` (`;`)
  * `TOKEN_EOF`, `TOKEN_ERROR`
* **Implementation Details:** Operates directly on `const char*` script buffers without heap allocation.

---

### 2. Symbol Table & Built-in Dispatch Engine
* **Target:** `kernel/serpentc/env.h`, `kernel/serpentc/builtins.c`
* **Variable Storage:** Simple static symbol table supporting integer variables (`int32_t / uint32_t`).
* **Built-in Graphics Primitives:**
  * `feather_draw_line(x0, y0, x1, y1, color)` -> Dispatches to `vbe_draw_line`
  * `feather_rect(x, y, w, h, color)` -> Dispatches to `vbe_fill_rect`
  * `feather_clear(color)` -> Dispatches to `vbe_clear`
  * `tonal_now()` -> Dispatches to `pit_get_ticks`

---

### 3. Recursive Statement Parser & Interpreter
* **Target:** `kernel/serpentc/serpentc.c`, `kernel/serpentc/serpentc.h`
* **Grammar:**
  ```ebnf
  script      ::= statement*
  statement   ::= assignment ';' | call ';'
  assignment  ::= IDENTIFIER '=' expression
  call        ::= IDENTIFIER '(' [expression (',' expression)*] ')'
  expression  ::= INT_LITERAL | IDENTIFIER
  ```
* **Execution Flow:** `serpentc_eval(script)` tokenizes, parses AST/statements, evaluates expressions, and dispatches native C function calls.

---

## File Modification & Addition Summary

| File Path | Status | Description |
|---|---|---|
| [kernel/serpentc/lexer.h](file:///Users/openclaw/Github/QuetzalOS/kernel/serpentc/lexer.h) | **NEW** | Lexer token types and scanner state definition |
| [kernel/serpentc/lexer.c](file:///Users/openclaw/Github/QuetzalOS/kernel/serpentc/lexer.c) | **NEW** | Freestanding string tokenizer |
| [kernel/serpentc/builtins.h](file:///Users/openclaw/Github/QuetzalOS/kernel/serpentc/builtins.h) | **NEW** | Symbol table and built-in dispatch declarations |
| [kernel/serpentc/builtins.c](file:///Users/openclaw/Github/QuetzalOS/kernel/serpentc/builtins.c) | **NEW** | Built-in graphics bindings and variable environment |
| [kernel/serpentc/serpentc.c](file:///Users/openclaw/Github/QuetzalOS/kernel/serpentc/serpentc.c) | **MODIFY** | Parser, interpreter loop, and `serpentc_eval` implementation |
| [Makefile](file:///Users/openclaw/Github/QuetzalOS/Makefile) | **MODIFY** | Include new SerpentC C source files in build target |

---

## Verification Plan

### Automated Build Verification
* Run `make clean && make` to ensure all new SerpentC modules compile cleanly without warnings.

### Runtime Verification
* Execute `./qemu.sh` in QEMU.
* Pass SerpentC script strings (e.g. `feather_rect(650, 480, 100, 50, 0xFFD700); feather_draw_line(0, 500, 800, 500, 0x00FFFF);`) in `kernel_main` and confirm graphics render dynamically on screen via SerpentC evaluation!
