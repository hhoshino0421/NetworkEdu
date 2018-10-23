//
// Created by hhoshino on 18/10/21.
//
#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/ioctl.h>
#include <netpacket/packet.h>
#include <netinet/ip_icmp.h>
#include <netinet/if_ether.h>
#include <netinet/udp.h>
#include <linux/if.h>
#include <arpa/inet.h>
#include <pthread.h>
#include "sock.h"
#include "ether.h"
#include "ip.h"
#include "icmp.h"
#include "dhcp.h"
#include "udp.h"
#include "param.h"

extern PARAM    Param;

struct pseudo_ip {
    struct in_addr  ip_src;
    struct in_addr  ip_dst;
    u_int8_t        dummy;
    u_int8_t        ip_p;
    u_int16_t       ip_len;
};

#define     UDP_TABLE_NO    (16)

typedef struct {
    u_int16_t port;
} UDP_TABLE;

UDP_TABLE       UpdTable[UDP_TABLE_NO];

pthread_rwlock_t    UpdTableLock = PTHREAD_RWLOCK_INITIALIZER;


int print_udp(struct udphr *udp) {
    printf("udp-------------------------------------------------------\n");
    
}