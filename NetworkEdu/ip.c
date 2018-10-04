//
// Created by hhoshino on 18/10/02.
//

#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <sys/ioctl.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <arpa/inet.h>
#include <net/ethernet.h>   //add Hoshino Hitoshi
#include "sock.h"
#include "ether.h"
#include "arp.h"
#include "ip.h"
#include "icmp.h"
#include "param.h"
#include "Common.h"

extern PARAM    Param;

#define     IP_RECV_BUF_NO  (16)

typedef struct {
    time_t      timestamp;
    int         id;
    u_int8_t    data[64 * 1024];
    int         len;
} IP_RECV_BUF;

IP_RECV_BUF     IpRecvBuf[IP_RECV_BUF_NO];


void print_ip(struct ip *ip) {

    static char *proto[] = {
            "undefined",
            "ICMP",
            "IGMP",
            "undefined",
            "IPIP",
            "undefined",
            "TCP",
            "undefined",
            "EGP",
            "undefined",
            "undefined",
            "undefined",
            "PUP",
            "undefined",
            "undefined",
            "undefined",
            "undefined",
            "UDP"
    };

    char buf1[80];

    printf("ip(start)--------------------------------------------------------------------------------------\n");

    printf("ip_v=%u\n", ip->ip_v);
    printf("ip_hl=%u\n", ip->ip_hl);
    printf("ip_tos=%x\n", ip->ip_tos);
    printf("ip_len=%d\n", ntohs(ip->ip_len));
    printf("ip_id=%u\n", ntohs(ip->ip_id));
    printf("ip_off=%x,%d\n", (ntohs(ip->ip_off)) >> 13 & 0x07, ntohs(ip->ip_off) & IP_OFFMASK);
    printf("ip_ttl=%u\n", ip->ip_ttl);
    printf("ip_p=%u\n", ip->ip_p);

    if (ip->ip_p <= 17) {
        printf("(%s)\n", proto[ip->ip_p]);
    } else {
        printf("(undefined)\n");
    }

    printf("ip_sum=%04x\n", ntohs(ip->ip_sum));
    printf("ip_src=%s\n", inet_ntop(AF_INET, &ip->ip_src, buf1, sizeof(buf1)));
    printf("ip_dst=%s\n", inet_ntop(AF_INET, &ip->ip_dst, buf1, sizeof(buf1)));

    printf("ip(end)----------------------------------------------------------------------------------------\n");

    return;

}

int IpRecvBufInit() {

    int     i;

    for (i = 0; i < IP_RECV_BUF_NO; i++) {
        IpRecvBuf[i].id = -1;
    }

    return PROCESS_RESULT_SUCCESS;

}


int IpRecvBufAdd(u_int16_t id) {

    int     i,  freeNo,  oldestNo,  intoNo;
    time_t  oldestTime;

    freeNo      = -1;
    oldestTime  = ULONG_MAX;
    oldestNo    = -1;

    for (i = 0; i < IP_RECV_BUF_NO; i++) {

        if (IpRecvBuf[i].id == -1) {

            freeNo = i;

        } else {

            if (IpRecvBuf[i].id == id) {
                return (i);
            }

            if (IpRecvBuf[i].timestamp < oldestTime) {
                oldestTime = IpRecvBuf[i].timestamp;
                oldestNo = i;
            }

        }

    }


    if (freeNo==-1) {
        intoNo = oldestNo;
    } else {
        intoNo = freeNo;
    }

    IpRecvBuf[intoNo].timestamp = time(NULL);
    IpRecvBuf[intoNo].id        = id;
    IpRecvBuf[intoNo].len       = 0;

    return (intoNo);

}


int IpRecvBufDel(u_int16_t id) {

    int     i;

    for (i = 0; i < IP_RECV_BUF_NO; i++) {

        if (IpRecvBuf[i].id == id){
            IpRecvBuf[i].id = -1;
            return 1;
        }
    }

    return 0;
}


int IpRecvBufSearch(u_int16_t id) {

    int     i;

    for (i = 0; i < IP_RECV_BUF_NO; i++) {

        if (IpRecvBuf[i].id == id) {
            return  1;
        }
    }

    return -1;
}


int IpRecv(int soc, u_int8_t *raw, int raw_len, struct ether_header *eh, u_int8_t *data, int len) {

    struct  ip      *ip;
    u_int8_t        option[1500];
    u_int16_t       sum;
    int             optionLen, no, off, plen;

    u_int8_t        *ptr = data;

    if (len < (int)sizeof(struct ip)) {

        printf("len(%d)<sizeof(struct ip)\n", len);
        return  PROCESS_RESULT_ERROR;

    }

    ip = (struct ip *)ptr;
    ptr += sizeof(struct ip);
    len -= sizeof(struct ip);
    optionLen = ip->ip_hl * 4 - sizeof(struct ip);

    if (optionLen > 0) {

        if (optionLen >= 1500) {
            printf("IP optionLen(%d) too big\n", optionLen);
            return PROCESS_RESULT_ERROR;
        }

        memcpy(option, ptr, optionLen);
        ptr += optionLen;
        len -= optionLen;
    }

    if (optionLen == 0) {
        sum = checksum((u_int8_t *)ip, sizeof(struct ip));
    } else {
        sum = checksum2((u_int8_t *)ip, sizeof(struct ip), option, optionLen);
    }

    if (sum != 0 && sum != 0xFFFF) {
        printf("bad ip checksum\n");
        return PROCESS_RESULT_ERROR;
    }

    plen = ntohs(ip->ip_len) - ip->ip_hl * 4;
    no = IpRecvBufAdd(ntohs(ip->ip_id));
    off = (ntohs(ip->ip_off) & IP_OFFMASK) * 8;
    memcpy(IpRecvBuf[no].data + off, ptr, plen);

    if (!(ntohs(ip->ip_off)&IP_MF)) {

        IpRecvBuf[no].len = off + plen;

        if (ip->ip_p == IPPROTO_ICMP) {

            IcmpRecv(soc, raw, raw_len, eh, ip, IpRecvBuf[no].data, IpRecvBuf[no].len);

        }

        IpRecvBufDel(ntohs(ip->ip_id));
    }

    return PROCESS_RESULT_SUCCESS;

}


int IpSendLink(int soc,u_int8_t smac[6],u_int8_t dmac[6]
                , struct in_addr *saddr, struct in_addr *daddr
                , u_int8_t proto,int dontFlagment
                , int ttl, u_int8_t *data, int len) {

    struct ip       *ip;
    u_int8_t        *dptr, *ptr, sbuf[ETHERMTU];
    u_int16_t       id;
    int             lest, sendLen, off, flagment;

    if (dontFlagment && len > Param.MTU - sizeof(struct ip)) {
        printf("IpSend:data too long:%d\n", len);
        return PROCESS_RESULT_ERROR;
    }

    id = random();

    dptr = data;
    lest = len;

    while(lest > 0) {

        if (lest>Param.MTU-sizeof(struct ip)) {
            sendLen = (Param.MTU - sizeof(struct ip)) / 8 * 8;
            flagment = 1;
        } else {
            sendLen = lest;
            flagment = 0;
        }

        ptr = sbuf;

        ip = (struct ip *)ptr;
        memset(ip, 0, sizeof(struct ip));
        ip->ip_v = 4;
        ip->ip_hl = 5;
        ip->ip_len = htons(sizeof(struct ip) + sendLen);
        ip->ip_id = htons(id);
        off = (dptr - data) / 8;

        if (dontFlagment) {
            ip->ip_off = htons(IP_DF);
        } else if (flagment) {
            ip->ip_off=htons(IP_MF);

        } else {
            ip->ip_off = htons((0) | (off & IP_OFFMASK));
        }

        ip->ip_ttl = ttl;
        ip->ip_p = proto;
        ip->ip_src.s_addr = saddr->s_addr;
        ip->ip_dst.s_addr = daddr->s_addr;
        ip->ip_sum = 0;
        ip->ip_sum = checksum((u_int8_t *)ip, sizeof(struct ip));
        ptr += sizeof(struct ip);

        memcpy(ptr, dptr, sendLen);
        ptr += sendLen;

        EtherSend(soc, smac, dmac, ETHERTYPE_IP, sbuf, (ptr - sbuf));

        print_ip(ip);

        dptr += sendLen;
        lest -= sendLen;

    }

    return PROCESS_RESULT_SUCCESS;

}

int IpSend(int soc, struct in_addr *saddr, struct in_addr *daddr
            ,u_int8_t proto, int dontFlagment, int ttl, u_int8_t *data, int len) {

    u_int8_t    dmac[6];
    char        buf1[80];
    int         ret;

    ret = 0;

    if (GetTargetMac(soc, daddr, dmac, 0)) {

        ret = IpSendLink(soc, Param.vmac, dmac, saddr, daddr
                , proto, dontFlagment, ttl, data, len);

    } else {

        printf("IpSend:%s Destination Host Unreachable\n", inet_ntop(AF_INET, daddr, buf1, sizeof(buf1)));
        ret =- 1;
    }

    return ret;

}