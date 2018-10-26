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

UDP_TABLE           UdpTable[UDP_TABLE_NO];

pthread_rwlock_t    UdpTableLock = PTHREAD_RWLOCK_INITIALIZER;


int print_udp(struct udphdr *udp) {

    printf("udp-------------------------------------------------------\n");
    printf("source=%d\n", ntohs(udp->source));
    printf("dest=%d\n", ntohs(udp->dest));
    printf("len=%d\n", ntohs(udp->len));
    printf("check=04x\n", ntohs(udp->check));

    return 0;

}


u_int16_t UdpChecksum(struct in_addr *saddr, struct in_addr *daddr
                    , u_int8_t proto, u_int8_t *data, int len) {


    struct pseudo_ip    p_ip;
    u_int16_t           sum;

    memset(&p_ip, 0, sizeof(struct pseudo_ip));
    p_ip.ip_src.s_addr = saddr->s_addr;
    p_ip.ip_dst.s_addr = daddr->s_addr;
    p_ip.ip_p   = proto;
    p_ip.ip_len = htons(len);

    sum = checksum2((u_int8_t *)&p_ip, sizeof(struct pseudo_ip), data, len);

    if (sum == 0x0000) {
        sum = 0xFFFF;
    }

    return sum;

}


int UdpAddTable(u_int16_t port) {

    int     i, freeNo;

    pthread_rwlock_wrlock(&UdpTableLock);

    freeNo = -1;
    for (i = 0; i < UDP_TABLE_NO; i++) {

        if (UdpTable[i].port == port) {

            printf("UdpAddTable:port &d:already exist\n", port);
            pthread_rwlock_unlock(&UdpTableLock);
            return -1;

        } else if (UdpTable[i].port == 0){

            if (freeNo == -1) {
                freeNo = i;
            }

        }
    }

    if (freeNo == -1) {

        printf("UdpAddTable:no free table\n");
        pthread_rwlock_unlock(&UdpTableLock);
        return -1;

    }

    UdpTable[freeNo].port = port;

    pthread_rwlock_unlock(&UdpTableLock);

    return freeNo;

}


int UdpSearchTable(u_int16_t port) {

    int     i;

    pthread_rwlock_wrlock(&UdpTableLock);

    for (i = 0; i < UDP_TABLE_NO; i++) {

        if (UdpTable[i].port == port) {

            pthread_rwlock_unlock(&UdpTableLock);
            return (i);
        }

    }

    pthread_rwlock_unlock(&UdpTableLock);

    return -1;

}