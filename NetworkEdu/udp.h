//
// Created by hhoshino on 18/10/21.
//

#ifndef NETWORKEDU_UDP_H
#define NETWORKEDU_UDP_H

#include <sys/types.h>

int print_udp(struct udphdr *udp);
u_int16_t UdpChecksum(struct in_addr *sassr, struct in_addr *daddr, u_int8_t proto, u_int8_t *daaata, int len);
int UdpAddTable(u_int16_t port);
int UdpSearchTable(u_int16_t port);
int UdpShowTable();
u_int16_t UdpSearchFreePort();
int UdpSocket(u_int16_t port);
int UdpSocketClose(u_int16_t port);
int UdpSendLink(int soc, u_int8_t smac[6], u_int8_t dmac[6], struct in_addr *saddr, struct in_addr *daddr,
        u_int16_t sport, u_int16_t dport, int dontFlagment, u_int8_t *data, int len);
int UdpSend(int soc, struct in_addr *saddr, struct in_addr *daddr, u_int16_t sport, u_int16_t dport
       , int dontFlagment, u_int8_t *data, int len);
int UdpRecv(int soc, struct ether_header *eh, struct ip *ip, u_int8_t *data, int len);



#endif //NETWORKEDU_UDP_H