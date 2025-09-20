# CSC HW4 - CTF (Capture The Flag) 專案文檔

## 專案概述

CSC HW4 是一個綜合性的網路安全 CTF (Capture The Flag) 競賽專案，包含 7 個不同類型的漏洞挑戰。每個挑戰都設計用來測試參賽者在不同網路安全領域的技能，包括二進制漏洞利用、密碼學攻擊、記憶體安全、以及系統安全等。

## 挑戰類型分析

### 1. Password Checker (密碼檢查器)
- **漏洞類型**: 整數溢出 (Integer Overflow)
- **技術細節**: 使用 `int8_t` 類型儲存字串長度，當輸入超過 127 字元時會發生整數溢出，導致負數比較觸發 flag 顯示
- **攻擊向量**: 發送 256 字元的 payload 使 `strlen()` 返回值溢出為負數

### 2. Simple Shell (簡單殼層)
- **漏洞類型**: 緩衝區溢出 (Buffer Overflow) + 結構體覆蓋
- **技術細節**: 註冊功能中的 `strcpy()` 操作可能覆蓋相鄰的 `admin` 結構體，導致權限提升
- **攻擊向量**: 精心構造用戶名和密碼來覆蓋 `admin` 結構體，然後以 admin 身份登入執行系統命令

### 3. Simple ROP (簡單 ROP 攻擊)
- **漏洞類型**: 返回導向程式設計 (Return-Oriented Programming)
- **技術細節**: 經典的 ROP 鏈構造，利用現有的程式碼片段 (gadgets) 來執行 shellcode
- **攻擊向量**: 構造 ROP 鏈調用 `execve("/bin/sh", NULL, NULL)` 來獲得 shell 存取權限

### 4. Ret2Flag (返回至 Flag 函數)
- **漏洞類型**: 簡單的返回地址覆蓋
- **技術細節**: 直接覆蓋返回地址跳轉到 `putFlag()` 函數
- **攻擊向量**: 利用緩衝區溢出覆蓋返回地址，直接跳轉到目標函數

### 5. Secure Random (安全隨機數)
- **漏洞類型**: 密碼學弱點 (Cryptographic Weakness)
- **技術細節**: 使用 `time(NULL)` 作為隨機數種子，使得隨機數可預測
- **攻擊向量**: 重現相同的隨機數生成算法來預測輸出值

### 6. Simple RTOS (簡單即時作業系統)
- **漏洞類型**: 格式化字串漏洞 (Format String Vulnerability)
- **技術細節**: `printf(buf)` 直接輸出用戶輸入，沒有格式字串驗證
- **攻擊向量**: 利用格式化字串漏洞讀取或寫入記憶體

### 7. Hard ROP (困難 ROP 攻擊)
- **漏洞類型**: 複雜的 ROP 攻擊 + 多層緩衝區溢出
- **技術細節**: 需要處理多個保護機制，包括 NX、ASLR、Stack Canaries 等
- **攻擊向量**: 構造複雜的 ROP 鏈來繞過各種保護機制

## 技術細節與安全影響分析

此 CTF 專案涵蓋了現代軟體安全中最常見和危險的漏洞類型，包括記憶體安全問題（緩衝區溢出、格式化字串漏洞）、密碼學實現錯誤（可預測隨機數）、以及進階攻擊技術（ROP 攻擊）。這些漏洞可能導致任意程式碼執行、權限提升、敏感資料洩露、以及系統完全被攻陷。從安全影響角度來看，這些攻擊可能造成受害者的系統被完全控制、敏感資料被竊取、服務被中斷、以及可能被用作進一步攻擊的跳板。為了防護這些攻擊，可以採用現代編譯器保護機制（如 ASLR、DEP/NX、Stack Canaries、CFI）、安全的程式設計實踐（如邊界檢查、輸入驗證、安全的記憶體管理）、強密碼學實現（如使用加密安全的隨機數生成器）、以及持續的安全測試和程式碼審查來建立多層次的安全防護體系，確保應用程式的完整性和系統的安全性。

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

## 環境設置

### 1. 建構映像檔
```bash
docker compose build
```

### 2. 啟動容器環境
```bash
docker compose up -d
```

### 3. 連接容器
```bash
docker exec -it <container_name> bash
```

### 4. 停止環境
```bash
docker compose down
```

## 挑戰端口映射

- **Password Checker**: 30170
- **Secure Random**: 30171  
- **Simple Shell**: 30172
- **Simple ROP**: 30173
- **Ret2Flag**: 30174
- **Simple RTOS**: 30175
- **Hard ROP**: 30176

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

## 法律聲明

本專案僅供教育和學習目的使用。所有攻擊技術和漏洞利用方法都應該在合法的環境中進行測試，不得用於任何惡意目的。使用者需要遵守相關法律法規，並對自己的行為負責。

## 學習目標

通過完成這些 CTF 挑戰，學習者將能夠：

1. **理解常見漏洞類型**: 掌握緩衝區溢出、格式化字串、整數溢出等常見漏洞的原理
2. **學習攻擊技術**: 了解 ROP、格式化字串攻擊等進階攻擊技術
3. **掌握防護機制**: 學習現代編譯器保護機制和安全程式設計實踐
4. **提升安全意識**: 培養安全程式設計思維和漏洞分析能力
5. **實踐技能應用**: 在實際環境中應用所學的安全知識和技能

---

*此文檔提供了 CSC HW4 CTF 專案的全面概述，包括技術細節、安全影響分析、以及實用的防護建議。*
