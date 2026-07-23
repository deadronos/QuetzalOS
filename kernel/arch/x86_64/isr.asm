; ==============================================================================
; QuetzalOS (TonalOS) - Interrupt Service Routine Wrappers
; Save 64-bit caller registers -> call C handlers -> restore -> iretq
; ==============================================================================

BITS 64

global irq0_stub
global irq1_stub
global irq12_stub

extern pit_handler
extern keyboard_handler
extern mouse_handler

%macro PUSHALL 0
    push rax
    push rbx
    push rcx
    push rdx
    push rsi
    push rdi
    push rbp
    push r8
    push r9
    push r10
    push r11
    push r12
    push r13
    push r14
    push r15
%endmacro

%macro POPALL 0
    pop r15
    pop r14
    pop r13
    pop r12
    pop r11
    pop r10
    pop r9
    pop r8
    pop rbp
    pop rdi
    pop rsi
    pop rdx
    pop rcx
    pop rbx
    pop rax
%endmacro

section .text

irq0_stub:
    PUSHALL
    call pit_handler
    POPALL
    iretq

irq1_stub:
    PUSHALL
    call keyboard_handler
    POPALL
    iretq

irq12_stub:
    PUSHALL
    call mouse_handler
    POPALL
    iretq
