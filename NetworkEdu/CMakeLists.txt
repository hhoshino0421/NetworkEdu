cmake_minimum_required(VERSION 3.12)
project(NetworkEdu)

find_package(Threads REQUIRED)

set(CMAKE_CXX_STANDARD 14)

add_executable(NetworkEdu main.c param.h param.c Common.h sock.h sock.c ether.h ether.c arp.h arp.c ip.h ip.c icmp.h icmp.c cmd.h cmd.c udp.h udp.c dhcp.h dhcp.c)

target_link_libraries(NetworkEdu Threads::Threads)