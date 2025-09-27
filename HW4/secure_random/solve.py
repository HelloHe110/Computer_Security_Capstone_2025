#!/usr/bin/env python3
from pwn import *
import time
import ctypes
from datetime import datetime
import pytz

def unsigned_int_multiply(a, b):
    # Simulate C's unsigned int multiplication with 32-bit wrap-around
    return (a * b) & 0xFFFFFFFF

def long_secure_random(timestamp):
    # Set the same seed as the C program
    ctypes.CDLL('libc.so.6').srand(timestamp)
    
    # Generate the same random numbers
    r = [0] * 100
    for i in range(100):
        r[i] = ctypes.CDLL('libc.so.6').rand() % 32323
    
    # Apply the same transformation with C-style unsigned int arithmetic
    for i in range(1, 100):
        # Break down the calculation to handle unsigned int arithmetic properly
        term1 = unsigned_int_multiply(r[i], r[i-1])
        term1 = unsigned_int_multiply(term1, r[i-1])
        term1 = unsigned_int_multiply(term1, r[i-1])
        
        term2 = unsigned_int_multiply(r[i], r[i-1])
        term2 = unsigned_int_multiply(term2, r[i-1])
        term2 = unsigned_int_multiply(term2, 3)
        
        term3 = unsigned_int_multiply(r[i], r[i-1])
        term3 = unsigned_int_multiply(term3, 2)
        
        r[i] = (term1 + term2 + term3 + r[i]) & 0xFFFFFFFF
    
    return r[99]

def try_get_flag():
    # Set timezone to Asia/Taipei
    tz = pytz.timezone('Asia/Taipei')
    current_time = datetime.now(tz)
    base_timestamp = int(current_time.timestamp())
    
    # Try timestamps around the current time
    for offset in range(-2, 3):  # Try -2, -1, 0, 1, 2 seconds
        timestamp = base_timestamp + offset
        predicted = long_secure_random(timestamp)
        
        try:
            # Connect to server
            p = remote('140.113.207.245', 30171)
            p.recvuntil(b"Try to break my unbreakable secure random\n")
            
            # Send the predicted number
            p.sendline(str(predicted).encode())
            
            # Get response
            response = p.recvline().decode().strip()
            
            if "flag" in response.lower():
                flag = p.recvline().decode().strip()
                print(f"Success with timestamp offset {offset}")
                print(f"\033[93;1mFlag: {flag}\033[0m")
                p.close()
                return True
            else:
                print(f"Failed with timestamp offset {offset}")
            
            p.close()
        except Exception as e:
            print(f"Error with timestamp offset {offset}: {str(e)}")
            try:
                p.close()
            except:
                pass
    
    return False

# Try to get the flag
if not try_get_flag():
    print("Failed to get the flag after trying all timestamps")
