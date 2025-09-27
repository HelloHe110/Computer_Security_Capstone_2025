#!/usr/bin/env python3
from pwn import *

# Specify the path to your binary; update accordingly.
binary_path = "./simple_rtos"
elf = ELF(binary_path)

# Print out all symbols (functions and global variables)
print("=== All Symbols in Binary ===")
for symbol, address in elf.symbols.items():
    print(f"{symbol:20}: {hex(address)}")
