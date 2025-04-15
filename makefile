CC=g++
CFLAGS=-Wall -Wextra -std=c++17
LIBS=-lpthread -lnetfilter_queue -ltins

all: icmp_redirect
	 echo 1 | sudo tee /proc/sys/net/ipv4/ip_forward
	 echo 0 | sudo tee /proc/sys/net/ipv4/conf/*/send_redirects
Force:

icmp_redirect:  Force
	sudo $(CC) $(CFLAGS) -o icmp_redirect icmp_redirect.cpp $(LIBS)
