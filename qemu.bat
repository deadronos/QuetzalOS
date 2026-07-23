@echo off
rem Run QuetzalOS in QEMU x86_64 on Windows
qemu-system-x86_64.exe ^
    -kernel quetzal.bin ^
    -vga std ^
    -m 512M ^
    -no-reboot ^
    -no-shutdown ^
    -serial stdio
