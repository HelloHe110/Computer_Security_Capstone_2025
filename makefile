CC=g++
CFLAGS=-Wall -Wextra -std=c++17
LIBS=-lpthread -lnetfilter_queue -ltins

all: icmp_redirect dns_spoof hanyu_pharm dora_pharm
	 echo 1 | sudo tee /proc/sys/net/ipv4/ip_forward
	 echo 0 | sudo tee /proc/sys/net/ipv4/conf/*/send_redirects
	 sudo iptables -I FORWARD -p udp --dport 53 -j NFQUEUE --queue-num 0
Force:

icmp_redirect:  Force
	sudo $(CC) $(CFLAGS) -o icmp_redirect icmp_redirect.cpp $(LIBS)

dns_spoof:  Force
	sudo $(CC) $(CFLAGS) -o dns_spoof dns_spoof.cpp $(LIBS)

hanyu_pharm:  Force
	sudo $(CC) $(CFLAGS) -o hanyu_pharm hanyu_pharm.cpp $(LIBS)

dora_pharm:  Force
	sudo $(CC) $(CFLAGS) -o dora_pharm dora_pharm.cpp $(LIBS)

clean:
	rm -f icmp_redirect dns_spoof hanyu_pharm dora_pharm