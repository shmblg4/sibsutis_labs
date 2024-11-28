#!/bin/bash

echo -e "\e[1;32mНазвание и версия ОС:\e[m\n"
lsb_release -a

echo -e "\n\e[1;32mВерсия и архитектура ядра Linux:\e[m"
uname -r && uname -m

echo -e "\n\e[1;32mИнформация о процессоре:\e[m"
lscpu | grep -i -e 'model name' -e 'architecture' -e 'cpu(s)' -e 'thread(s) per core' -e 'core(s) per socket' -e 'socket(s)'

echo -e "\n\e[1;32mИнформация об оперативной памяти:\e[m"
free -h

echo -e "\n\e[1;32mИнформация о сетевых интерфейсах:\e[m"
for iface in $(ls /sys/class/net/); do
    echo "Интерфейс: $iface"
    ip a show $iface | grep -E 'inet|link/ether'
    ethtool "$iface" | grep "Speed"
    echo
done
speedtest
iperf -c 127.0.0.1

echo -e "\e[1;32mИнформация о системных разделах:\e[m"
df -h

echo -e "\n\e[1;32mКоличество процессоров:\e[m"
nproc

