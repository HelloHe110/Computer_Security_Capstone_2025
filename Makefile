CC = gcc
CFLAGS = -O2 -Wall -Wno-error
TARGETS = /app/aes-tool attack_server crack_attack

all: $(TARGETS)
	chmod +x attack_server
	chmod +x crack_attack

# **直接編譯到 `/app/aes-tool`**
/app/aes-tool: /app/aes-tool.c
	$(CC) $(CFLAGS) -o /app/aes-tool /app/aes-tool.c -lssl -lcrypto

    
