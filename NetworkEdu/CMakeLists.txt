cmake_minimum_required(VERSION 3.12)
project(NetworkEdu)

set(CMAKE_CXX_STANDARD 14)

add_executable(NetworkEdu main.c param.h param.c Common.h sock.h sock.c ether.h ether.c arp.h arp.c)