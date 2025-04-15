echo 1 | sudo tee /proc/sys/net/ipv4/conf/all/accept_redirects
echo 1 | sudo tee /proc/sys/net/ipv4/conf/default/accept_redirects
echo 1 | sudo tee /proc/sys/net/ipv4/conf/ens33/accept_redirects

# sudo iptables -A INPUT -p icmp -j DROP
# sudo iptables -A FORWARD -p icmp -j DROP
