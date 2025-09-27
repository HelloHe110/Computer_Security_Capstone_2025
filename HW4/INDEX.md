# CSC HW4 CTF 專案文檔索引

## 文檔概覽

本索引提供了 CSC HW4 CTF 專案所有文檔的完整導航，幫助學習者快速找到所需的技術資訊和學習資源。

## 主要文檔

### 1. 專案概述文檔
- **檔案**: `README_DOCUMENTATION.md`
- **內容**: 專案整體介紹、挑戰類型分析、環境設置、學習目標
- **適用對象**: 初學者、專案管理者
- **關鍵章節**:
  - 專案概述
  - 挑戰類型分析 (7個挑戰)
  - 技術細節與安全影響分析
  - 專案結構
  - 環境設置指南
  - 挑戰端口映射
  - 攻擊工具與技術
  - 防護措施
  - 法律聲明
  - 學習目標

### 2. 漏洞分析文檔
- **檔案**: `VULNERABILITY_ANALYSIS.md`
- **內容**: 詳細的漏洞技術分析、攻擊向量、防護機制
- **適用對象**: 安全研究員、滲透測試員、開發者
- **關鍵章節**:
  - Password Checker - 整數溢出漏洞
  - Simple Shell - 緩衝區溢出與結構體覆蓋
  - Simple ROP - 返回導向程式設計
  - Ret2Flag - 簡單返回地址覆蓋
  - Secure Random - 密碼學弱點
  - Simple RTOS - 格式化字串漏洞
  - Hard ROP - 複雜 ROP 攻擊
  - 通用防護策略
  - 檢測與響應

### 3. 漏洞利用與防護技術指南
- **檔案**: `EXPLOITATION_DEFENSE_GUIDE.md`
- **內容**: 完整的漏洞利用技術和防護機制實戰指南
- **適用對象**: 進階學習者、安全專家、系統管理員
- **關鍵章節**:
  - 漏洞利用技術 (5大類)
  - 防護機制 (編譯時、運行時、程式設計、系統級)
  - 實戰技巧 (漏洞發現、利用、防護繞過)
  - 工具與資源

## 挑戰詳細分析

### 1. Password Checker (密碼檢查器)
- **難度**: ⭐
- **漏洞類型**: 整數溢出
- **關鍵技術**: 整數溢出利用、邊界檢查繞過
- **相關文檔**: 
  - 概述: `README_DOCUMENTATION.md` - 挑戰類型分析
  - 技術分析: `VULNERABILITY_ANALYSIS.md` - 第1章
  - 利用技術: `EXPLOITATION_DEFENSE_GUIDE.md` - 第1部分第1節

### 2. Simple Shell (簡單殼層)
- **難度**: ⭐⭐
- **漏洞類型**: 緩衝區溢出 + 結構體覆蓋
- **關鍵技術**: 記憶體佈局分析、結構體覆蓋、權限提升
- **相關文檔**:
  - 概述: `README_DOCUMENTATION.md` - 挑戰類型分析
  - 技術分析: `VULNERABILITY_ANALYSIS.md` - 第2章
  - 利用技術: `EXPLOITATION_DEFENSE_GUIDE.md` - 第1部分第2節

### 3. Simple ROP (簡單 ROP 攻擊)
- **難度**: ⭐⭐⭐
- **漏洞類型**: 返回導向程式設計
- **關鍵技術**: ROP 鏈構造、Gadget 搜尋、系統調用
- **相關文檔**:
  - 概述: `README_DOCUMENTATION.md` - 挑戰類型分析
  - 技術分析: `VULNERABILITY_ANALYSIS.md` - 第3章
  - 利用技術: `EXPLOITATION_DEFENSE_GUIDE.md` - 第1部分第3節

### 4. Ret2Flag (返回至 Flag 函數)
- **難度**: ⭐⭐
- **漏洞類型**: 簡單返回地址覆蓋
- **關鍵技術**: 返回地址覆蓋、函數跳轉
- **相關文檔**:
  - 概述: `README_DOCUMENTATION.md` - 挑戰類型分析
  - 技術分析: `VULNERABILITY_ANALYSIS.md` - 第4章
  - 利用技術: `EXPLOITATION_DEFENSE_GUIDE.md` - 第1部分第2節

### 5. Secure Random (安全隨機數)
- **難度**: ⭐⭐
- **漏洞類型**: 密碼學弱點
- **關鍵技術**: 隨機數預測、密碼學攻擊
- **相關文檔**:
  - 概述: `README_DOCUMENTATION.md` - 挑戰類型分析
  - 技術分析: `VULNERABILITY_ANALYSIS.md` - 第5章
  - 利用技術: `EXPLOITATION_DEFENSE_GUIDE.md` - 第1部分第5節

### 6. Simple RTOS (簡單即時作業系統)
- **難度**: ⭐⭐⭐
- **漏洞類型**: 格式化字串漏洞
- **關鍵技術**: 格式化字串利用、記憶體讀寫、GOT 劫持
- **相關文檔**:
  - 概述: `README_DOCUMENTATION.md` - 挑戰類型分析
  - 技術分析: `VULNERABILITY_ANALYSIS.md` - 第6章
  - 利用技術: `EXPLOITATION_DEFENSE_GUIDE.md` - 第1部分第4節

### 7. Hard ROP (困難 ROP 攻擊)
- **難度**: ⭐⭐⭐⭐⭐
- **漏洞類型**: 複雜 ROP 攻擊
- **關鍵技術**: 多層保護繞過、記憶體洩露、複雜 ROP 鏈
- **相關文檔**:
  - 概述: `README_DOCUMENTATION.md` - 挑戰類型分析
  - 技術分析: `VULNERABILITY_ANALYSIS.md` - 第7章
  - 利用技術: `EXPLOITATION_DEFENSE_GUIDE.md` - 第1部分第3節

## 技術主題索引

### 漏洞類型
- **整數溢出**: `VULNERABILITY_ANALYSIS.md` - 第1章
- **緩衝區溢出**: `VULNERABILITY_ANALYSIS.md` - 第2、4、7章
- **ROP 攻擊**: `VULNERABILITY_ANALYSIS.md` - 第3、7章
- **格式化字串**: `VULNERABILITY_ANALYSIS.md` - 第6章
- **密碼學弱點**: `VULNERABILITY_ANALYSIS.md` - 第5章

### 攻擊技術
- **記憶體洩露**: `EXPLOITATION_DEFENSE_GUIDE.md` - 第1部分第3節
- **Gadget 搜尋**: `EXPLOITATION_DEFENSE_GUIDE.md` - 第1部分第3節
- **ROP 鏈構造**: `EXPLOITATION_DEFENSE_GUIDE.md` - 第1部分第3節
- **格式化字串利用**: `EXPLOITATION_DEFENSE_GUIDE.md` - 第1部分第4節
- **密碼學攻擊**: `EXPLOITATION_DEFENSE_GUIDE.md` - 第1部分第5節

### 防護機制
- **編譯時保護**: `EXPLOITATION_DEFENSE_GUIDE.md` - 第2部分第1節
- **運行時保護**: `EXPLOITATION_DEFENSE_GUIDE.md` - 第2部分第2節
- **程式設計最佳實踐**: `EXPLOITATION_DEFENSE_GUIDE.md` - 第2部分第3節
- **系統級防護**: `EXPLOITATION_DEFENSE_GUIDE.md` - 第2部分第4節

### 工具與資源
- **漏洞利用工具**: `EXPLOITATION_DEFENSE_GUIDE.md` - 第4部分第1節
- **防護工具**: `EXPLOITATION_DEFENSE_GUIDE.md` - 第4部分第2節
- **學習資源**: `EXPLOITATION_DEFENSE_GUIDE.md` - 第4部分第3節

## 學習路徑建議

### 初學者路徑
1. **開始**: `README_DOCUMENTATION.md` - 專案概述
2. **基礎**: `VULNERABILITY_ANALYSIS.md` - 第1、2、4、5章
3. **進階**: `VULNERABILITY_ANALYSIS.md` - 第3、6、7章
4. **實戰**: `EXPLOITATION_DEFENSE_GUIDE.md` - 第1部分

### 進階學習者路徑
1. **深入分析**: `VULNERABILITY_ANALYSIS.md` - 全部章節
2. **技術實戰**: `EXPLOITATION_DEFENSE_GUIDE.md` - 第1、3部分
3. **防護機制**: `EXPLOITATION_DEFENSE_GUIDE.md` - 第2部分
4. **工具使用**: `EXPLOITATION_DEFENSE_GUIDE.md` - 第4部分

### 安全專家路徑
1. **全面理解**: 所有文檔的完整閱讀
2. **深度研究**: 特定漏洞類型的深入研究
3. **創新應用**: 結合實際環境的技術應用
4. **防護設計**: 基於文檔內容的防護體系設計

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

## 文檔維護

### 更新記錄
- **v1.0**: 初始版本，包含所有基礎文檔
- **v1.1**: 添加詳細的技術分析
- **v1.2**: 完善防護機制指南
- **v1.3**: 優化學習路徑和索引結構

### 貢獻指南
1. 遵循現有的文檔結構和格式
2. 確保技術內容的準確性和完整性
3. 提供清晰的代碼示例和解釋
4. 保持文檔的一致性和可讀性

### 反饋與建議
如有任何問題、建議或改進意見，請通過適當的渠道反饋，以便持續改進文檔品質。

---

*本索引提供了 CSC HW4 CTF 專案文檔的完整導航，幫助學習者根據自己的需求和水平選擇合適的學習路徑。*
