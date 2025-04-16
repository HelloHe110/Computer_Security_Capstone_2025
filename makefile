CC=g++
CFLAGS=-Wall -Wextra -std=c++17
LIBS=-lpthread -lnetfilter_queue -ltins

all: icmp_redirect hanyu_pharm he110_pharm
	 echo 1 | sudo tee /proc/sys/net/ipv4/ip_forward
	 echo 0 | sudo tee /proc/sys/net/ipv4/conf/*/send_redirects
	 sudo iptables -F
	 sudo iptables -I FORWARD -p udp --dport 53 -j NFQUEUE --queue-num 0
Force:

icmp_redirect:  Force
	sudo $(CC) $(CFLAGS) -o icmp_redirect icmp_redirect.cpp $(LIBS)

hanyu_pharm:  Force
	sudo $(CC) $(CFLAGS) -o hanyu_pharm hanyu_pharm.cpp $(LIBS)

he110_pharm:  Force
	sudo $(CC) $(CFLAGS) -o he110_pharm he110_pharm.cpp $(LIBS)

clean:
	rm -f icmp_redirect hanyu_pharm he110_pharm