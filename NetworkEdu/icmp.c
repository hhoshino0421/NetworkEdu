//
// Created by hhoshino on 18/10/03.
//

#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <sys/time.h>
#include <netinet/ip_icmp.h>
#include <netinet/if_ether.h>
#include <arpa/inet.h>
#include "sock.h"
#include "ether.h"
#include "ip.h"
#include "icmp.h"
#include "param.h"
#include "Common.h"

extern PARAM    Param;

#define ECHO_HDR_SIZE       (8)
#define PING_SEND_NO        (4)

typedef struct {
    struct timeval  sendTime;
} PING_DATA;

PING_DATA   PingData[PING_SEND_NO];


int print_icmp(struct icmp *icmp) {

    static char     *icmp_type[] = {
            "Echo Reply",
            "undefined",
            "undefined",
            "Destination Unreachable",
            "Source Quench",
            "Redirect",
            "undefined",
            "undefined",
            "Echo Request",
            "Router Adverisement",
            "Router Selection",
            "Time Exceeded for Datagram",
            "Parameter Problem on Datagram",
            "Timestamp Request",
            "Timestamp Reply",
            "Information Request",
            "Information Reply",
            "Address Mask Request",
            "Address Mask Reply"
    };

    printf("icmp(start)----------------------------------------------------------------------\n");

    printf("icmp_type=%u",icmp->icmp_type);

    if (icmp->icmp_type<=18) {
        printf("(%s)\n", icmp_type[icmp->icmp_type]);
    } else {
        printf("(undefined)\n");
    }

    printf("icmp_code=%u\n", icmp->icmp_code);
    printf("icmp_cksum=%u\n", ntohs(icmp->icmp_cksum));

    if (icmp->icmp_type == 0 || icmp->icmp_type == 8) {
        printf("icmp_id=%d\n", ntohs(icmp->icmp_id));
        printf("icmp_seq%u\n", ntohs(icmp->icmp_seq));
    }


    printf("icmp(end)------------------------------------------------------------------------\n");

    return PROCESS_RESULT_SUCCESS;

}


int IcmpSendEchoReply(int soc, struct ip *r_ip, struct icmp *r_icmp
                    , u_int8_t *data, int len, int ip_ttl) {


    u_int8_t    *ptr;
    u_int8_t    sbuf[64 * 1024];
    struct      icmp    *icmp;

    ptr = sbuf;
    icmp = (struct icmp *)ptr;

    memset(icmp, 0, sizeof(struct icmp));
    icmp->icmp_type = ICMP_ECHOREPLY;
    icmp->icmp_code = 0;
    icmp->icmp_hun.ih_idseq.icd_id = r_icmp->icmp_hun.ih_idseq.icd_id;
    icmp->icmp_hun.ih_idseq.icd_seq = r_icmp->icmp_hun.ih_idseq.icd_seq;
    icmp->icmp_cksum = 0;

    ptr += ECHO_HDR_SIZE;
    memcpy(ptr, data, len);
    ptr += len;

    icmp->icmp_cksum = checksum(sbuf, (ptr - sbuf));

    printf("=== ICMP reply ===[\n");

    IpSend(soc, &r_ip->ip_dst, &r_ip->ip_src, IPPROTO_ICMP, 0, ip_ttl, sbuf, (ptr-sbuf));

    print_icmp(icmp);

    printf("]\n");

    return PROCESS_RESULT_SUCCESS;

}


int IcmpSendEcho(int soc, struct in_addr *daddr, int seqNo, int size) {

    int         i, psize;
    u_int8_t    *ptr;
    u_int8_t    sbuf[64 * 1024];
    struct      icmp    *icmp;

    ptr = sbuf;
    icmp = (struct icmp *)ptr;
    memset(icmp, 0, sizeof(struct icmp));
    icmp->icmp_type = ICMP_ECHO;
    icmp->icmp_code = 0;
    icmp->icmp_hun.ih_idseq.icd_id = htons((u_int16_t)getpid());
    icmp->icmp_hun.ih_idseq.icd_seq = htons((u_int16_t)seqNo);
    icmp->icmp_cksum = 0;
    ptr += ECHO_HDR_SIZE;
    psize = (size - ECHO_HDR_SIZE);

    for (i = 0; i < psize; i++) {
        *ptr = (i & 0xFF);
        ptr++;
    }

    icmp->icmp_cksum = checksum((u_int8_t *)sbuf, (ptr - sbuf));

    printf("=== ICMP echo ===[\n");

    IpSend(soc, &Param.vip, daddr, IPPROTO_ICMP, 0, Param.IpTTL, sbuf, (ptr - sbuf));

    print_icmp(icmp);

    printf("]\n");

    gettimeofday(&PingData[seqNo-1].sendTime, NULL);

    return PROCESS_RESULT_SUCCESS;

}