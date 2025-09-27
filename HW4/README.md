# CSC HW4 - CTF (Capture The Flag) 專案

## 專案概述

CSC HW4 是一個綜合性的網路安全 CTF (Capture The Flag) 競賽專案，包含 7 個不同類型的漏洞挑戰。每個挑戰都設計用來測試參賽者在不同網路安全領域的技能，包括二進制漏洞利用、密碼學攻擊、記憶體安全、以及系統安全等。

## 挑戰類型分析

**詳細技術分析請參考**: [VULNERABILITY_ANALYSIS.md](./VULNERABILITY_ANALYSIS.md)

### 1. Password Checker (密碼檢查器) - ⭐
- **漏洞類型**: 整數溢出 (Integer Overflow)
- **技術細節**: 使用 `int8_t` 類型儲存字串長度，當輸入超過 127 字元時會發生整數溢出，導致負數比較觸發 flag 顯示
- **攻擊向量**: 發送 256 字元的 payload 使 `strlen()` 返回值溢出為負數
- **端口**: 30170

### 2. Simple Shell (簡單殼層) - ⭐⭐
- **漏洞類型**: 緩衝區溢出 (Buffer Overflow) + 結構體覆蓋
- **技術細節**: 註冊功能中的 `strcpy()` 操作可能覆蓋相鄰的 `admin` 結構體，導致權限提升
- **攻擊向量**: 精心構造用戶名和密碼來覆蓋 `admin` 結構體，然後以 admin 身份登入執行系統命令
- **端口**: 30172

### 3. Simple ROP (簡單 ROP 攻擊) - ⭐⭐⭐
- **漏洞類型**: 返回導向程式設計 (Return-Oriented Programming)
- **技術細節**: 經典的 ROP 鏈構造，利用現有的程式碼片段 (gadgets) 來執行 shellcode
- **攻擊向量**: 構造 ROP 鏈調用 `execve("/bin/sh", NULL, NULL)` 來獲得 shell 存取權限
- **端口**: 30173

### 4. Ret2Flag (返回至 Flag 函數) - ⭐⭐
- **漏洞類型**: 簡單的返回地址覆蓋
- **技術細節**: 直接覆蓋返回地址跳轉到 `putFlag()` 函數
- **攻擊向量**: 利用緩衝區溢出覆蓋返回地址，直接跳轉到目標函數
- **端口**: 30174

### 5. Secure Random (安全隨機數) - ⭐⭐
- **漏洞類型**: 密碼學弱點 (Cryptographic Weakness)
- **技術細節**: 使用 `time(NULL)` 作為隨機數種子，使得隨機數可預測
- **攻擊向量**: 重現相同的隨機數生成算法來預測輸出值
- **端口**: 30171

### 6. Simple RTOS (簡單即時作業系統) - ⭐⭐⭐
- **漏洞類型**: 格式化字串漏洞 (Format String Vulnerability)
- **技術細節**: `printf(buf)` 直接輸出用戶輸入，沒有格式字串驗證
- **攻擊向量**: 利用格式化字串漏洞讀取或寫入記憶體
- **端口**: 30175

### 7. Hard ROP (困難 ROP 攻擊) - ⭐⭐⭐⭐⭐
- **漏洞類型**: 複雜的 ROP 攻擊 + 多層緩衝區溢出
- **技術細節**: 需要處理多個保護機制，包括 NX、ASLR、Stack Canaries 等
- **攻擊向量**: 構造複雜的 ROP 鏈來繞過各種保護機制
- **端口**: 30176

## 專案結構

```
csc_hw4/
├── docker-compose.yaml          # Docker 容器編排配置
├── cmd.md                       # 命令使用說明
├── README.md                    # 基本專案說明
├── 2025csc-project4-ctf.pdf     # 專案需求文檔
├── password_checker/            # 密碼檢查器挑戰
├── simple_shell/               # 簡單殼層挑戰
├── simple_rop/                 # 簡單 ROP 挑戰
├── ret2flag/                   # 返回至 Flag 挑戰
├── secure_random/              # 安全隨機數挑戰
├── simple_rtos/                # 簡單 RTOS 挑戰
└── hard_rop/                   # 困難 ROP 挑戰
```

## 快速開始

### 環境設置

#### 1. 建構映像檔
```bash
docker compose build
```

#### 2. 啟動容器環境
```bash
docker compose up -d
```

#### 3. 連接容器
```bash
docker exec -it <container_name> bash
```

#### 4. 停止環境
```bash
docker compose down
```

### 挑戰端口映射

- **Password Checker**: 30170
- **Secure Random**: 30171  
- **Simple Shell**: 30172
- **Simple ROP**: 30173
- **Ret2Flag**: 30174
- **Simple RTOS**: 30175
- **Hard ROP**: 30176

## 技術細節與安全影響分析

此 CTF 專案涵蓋了現代軟體安全中最常見和危險的漏洞類型，包括記憶體安全問題（緩衝區溢出、格式化字串漏洞）、密碼學實現錯誤（可預測隨機數）、以及進階攻擊技術（ROP 攻擊）。這些漏洞可能導致任意程式碼執行、權限提升、敏感資料洩露、以及系統完全被攻陷。

從安全影響角度來看，這些攻擊可能造成受害者的系統被完全控制、敏感資料被竊取、服務被中斷、以及可能被用作進一步攻擊的跳板。

## 攻擊工具與技術

### 常用工具
- **pwntools**: Python 漏洞利用框架
- **ROPgadget**: ROP gadgets 搜尋工具
- **gdb**: GNU 除錯器
- **objdump**: 二進制檔案分析工具
- **strings**: 字串提取工具

### 攻擊技術
- **Buffer Overflow**: 緩衝區溢出攻擊
- **ROP (Return-Oriented Programming)**: 返回導向程式設計
- **Format String Attack**: 格式化字串攻擊
- **Integer Overflow**: 整數溢出攻擊
- **Cryptographic Attacks**: 密碼學攻擊

## 漏洞利用技術

### 1. 整數溢出利用

#### 技術原理
整數溢出發生在算術運算結果超出資料類型所能表示的範圍時。

#### 利用示例
```c
// 漏洞代碼
int8_t len;
char buffer[256];
fgets(buffer, 256, stdin);
len = strlen(buffer);  // 可能溢出

// 利用方法
payload = b'A' * 256  // 觸發 strlen() 返回 256
         # 轉換為 int8_t 時變成 -128
         # 滿足 len < 0 的條件
```

### 2. 緩衝區溢出利用

#### 技術原理
緩衝區溢出是最經典的記憶體安全漏洞，發生在程式向緩衝區寫入超過其容量的資料時。

#### 利用技術
- **返回地址覆蓋**: 覆蓋函數返回地址跳轉到惡意代碼
- **結構體覆蓋**: 覆蓋相鄰的資料結構
- **ROP 攻擊**: 利用現有代碼片段構造攻擊鏈

### 3. ROP 攻擊

#### 技術原理
ROP (Return-Oriented Programming) 是一種進階的攻擊技術，利用現有的程式碼片段 (gadgets) 來構造攻擊鏈。

#### 利用步驟
1. **Gadget 搜尋**: 尋找可用的程式碼片段
2. **鏈構造**: 將 gadgets 組合成攻擊鏈
3. **參數設置**: 設置系統調用參數
4. **執行**: 執行構造的 ROP 鏈

### 4. 格式化字串攻擊

#### 技術原理
格式化字串漏洞發生在程式直接將用戶輸入作為格式化字串參數傳遞給 `printf` 等函數時。

#### 利用技術
- **記憶體讀取**: 使用 `%x` 等格式符讀取記憶體
- **記憶體寫入**: 使用 `%n` 格式符寫入記憶體
- **GOT 劫持**: 修改 GOT 表項來劫持函數調用

## 防護措施

### 編譯器保護
- **ASLR (Address Space Layout Randomization)**: 地址空間隨機化
- **DEP/NX (Data Execution Prevention)**: 資料執行防護
- **Stack Canaries**: 堆疊保護機制
- **CFI (Control Flow Integrity)**: 控制流完整性

### 程式設計最佳實踐
- **輸入驗證**: 嚴格驗證所有用戶輸入
- **邊界檢查**: 確保陣列和緩衝區存取在合法範圍內
- **安全記憶體管理**: 使用安全的記憶體操作函數
- **密碼學安全**: 使用經過驗證的密碼學庫和算法

### 防護機制實現

#### 編譯時保護
```bash
# 啟用所有保護機制
gcc -fstack-protector-strong -fPIE -pie -Wl,-z,relro,-z,now -D_FORTIFY_SOURCE=2
```

#### 運行時保護
```c
// 使用安全的字串函數
strncpy(dest, src, sizeof(dest) - 1);
dest[sizeof(dest) - 1] = '\0';

// 邊界檢查
if (len < 0 || len >= MAX_SIZE) {
    return ERROR_INVALID_LENGTH;
}
```

## 學習路徑建議

### 初學者路徑
1. **開始**: 專案概述和基本概念
2. **基礎**: Password Checker, Ret2Flag, Secure Random
3. **進階**: Simple Shell, Simple ROP, Simple RTOS
4. **專家**: Hard ROP

### 進階學習者路徑
1. **深入分析**: 詳細的漏洞技術分析
2. **技術實戰**: 漏洞利用技術實踐
3. **防護機制**: 防護機制設計和實現
4. **工具使用**: 專業工具的使用

## 快速參考

### 常用命令
```bash
# 環境設置
docker compose build
docker compose up -d
docker exec -it <container_name> bash

# 編譯保護
gcc -fstack-protector-strong -fPIE -pie -Wl,-z,relro,-z,now

# 調試工具
gdb ./target
ROPgadget --binary ./target --only 'pop|ret'
```

### 關鍵概念
- **ASLR**: 地址空間隨機化
- **DEP/NX**: 資料執行防護
- **ROP**: 返回導向程式設計
- **CFI**: 控制流完整性
- **Canary**: 堆疊保護機制

### 重要工具
- **pwntools**: Python 漏洞利用框架
- **ROPgadget**: ROP gadgets 搜尋
- **AddressSanitizer**: 記憶體錯誤檢測
- **AFL**: 模糊測試工具

## 學習目標

通過完成這些 CTF 挑戰，學習者將能夠：

1. **理解常見漏洞類型**: 掌握緩衝區溢出、格式化字串、整數溢出等常見漏洞的原理
2. **學習攻擊技術**: 了解 ROP、格式化字串攻擊等進階攻擊技術
3. **掌握防護機制**: 學習現代編譯器保護機制和安全程式設計實踐
4. **提升安全意識**: 培養安全程式設計思維和漏洞分析能力
5. **實踐技能應用**: 在實際環境中應用所學的安全知識和技能

## 法律聲明

本專案僅供教育和學習目的使用。所有攻擊技術和漏洞利用方法都應該在合法的環境中進行測試，不得用於任何惡意目的。使用者需要遵守相關法律法規，並對自己的行為負責。