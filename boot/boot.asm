; ==============================================================================
; QuetzalOS (TonalOS) - Bootloader Entry Point
; Multiboot1 Compliant -> 32-bit Protected Mode -> 64-bit Long Mode Transition
; ==============================================================================

BITS 32

%define MULTIBOOT_MAGIC     0x1BADB002
%define MULTIBOOT_FLAGS     0x00000003   ; align modules + mem info
%define MULTIBOOT_CHECKSUM  -(MULTIBOOT_MAGIC + MULTIBOOT_FLAGS)

section .multiboot
align 4
    dd MULTIBOOT_MAGIC
    dd MULTIBOOT_FLAGS
    dd MULTIBOOT_CHECKSUM

section .bss
align 4096
pml4_table:
    resb 4096
pdpt_table:
    resb 4096
pd_tables:
    resb 4096 * 4               ; 4 Page Directories for 4 GB total identity mapping
stack_bottom:
    resb 65536                  ; 64 KB kernel boot stack
stack_top:

section .rodata
align 8
gdt64:
    dq 0 ; Null Descriptor
.code_descriptor: equ $ - gdt64
    dq (1<<43) | (1<<44) | (1<<47) | (1<<53) ; Code segment: Executable, Code/Data, Present, Long Mode
.data_descriptor: equ $ - gdt64
    dq (1<<41) | (1<<44) | (1<<47)           ; Data segment: Writable, Code/Data, Present
.pointer:
    dw $ - gdt64 - 1
    dq gdt64

section .text
global start
extern kernel_main

start:
    cli                         ; Disable interrupts
    mov esp, stack_top          ; Setup temporary 32-bit stack

    ; Check multiboot magic
    cmp eax, 0x2BADB002
    jne .no_multiboot

    ; Save multiboot info structure address (ebx)
    mov edi, ebx                ; Pass Multiboot info pointer as 1st arg to 64-bit main (RDI)

    ; Setup Page Tables (Identity Map full 4GB using 2MB pages)
    call setup_paging

    ; Enable PAE in CR4
    mov eax, cr4
    or eax, 1 << 5
    mov cr4, eax

    ; Load PML4 address into CR3
    mov eax, pml4_table
    mov cr3, eax

    ; Enable Long Mode in EFER MSR (0xC0000080)
    mov ecx, 0xC0000080
    rdmsr
    or eax, 1 << 8
    wrmsr

    ; Enable Paging in CR0 (Bit 31 PG, Bit 0 PE)
    mov eax, cr0
    or eax, (1 << 31) | (1 << 0)
    mov cr0, eax

    ; Load 64-bit GDT
    lgdt [gdt64.pointer]

    ; Far jump to 64-bit Long Mode Code Segment
    jmp gdt64.code_descriptor:long_mode_entry

.no_multiboot:
    hlt
    jmp .no_multiboot

setup_paging:
    ; Link PML4[0] -> PDPT
    mov eax, pdpt_table
    or eax, 0b11                ; Present + Writable
    mov [pml4_table], eax

    ; Link PDPT[0..3] -> 4 Page Directories (covering 4 GB)
    mov ecx, 0
.map_pdpt:
    mov eax, pd_tables
    mov edx, ecx
    shl edx, 12                 ; edx = ecx * 4096
    add eax, edx
    or eax, 0b11                ; Present + Writable
    mov [pdpt_table + ecx * 8], eax
    inc ecx
    cmp ecx, 4
    jne .map_pdpt

    ; Identity Map 2048 entries of 2MB = 4GB Total
    mov ecx, 0
.map_pd:
    mov eax, ecx
    shl eax, 21                 ; EAX = (ecx << 21) = low 32 bits of (ecx * 2MB)
    or eax, 0b10000011          ; Present + Writable + Page Size (2MB Huge Page)
    mov edx, ecx
    shr edx, 11                 ; EDX = (ecx >> 11) = high 32 bits of (ecx * 2MB)

    mov [pd_tables + ecx * 8], eax
    mov [pd_tables + ecx * 8 + 4], edx
    inc ecx
    cmp ecx, 2048               ; 2048 entries * 2MB = 4 GB
    jne .map_pd
    ret

BITS 64
long_mode_entry:
    ; Reload segment registers with 64-bit Data Segment
    mov ax, gdt64.data_descriptor
    mov ds, ax
    mov es, ax
    mov fs, ax
    mov gs, ax
    mov ss, ax

    ; Set 64-bit stack pointer
    mov rsp, stack_top

    ; Call 64-bit Kernel Main
    call kernel_main

.hang:
    cli
    hlt
    jmp .hang
