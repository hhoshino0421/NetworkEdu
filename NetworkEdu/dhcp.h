//
// Created by hhoshino on 18/11/04.
//

#include <unistd.h>

#ifndef NETWORKEDU_DHCP_H
#define NETWORKEDU_DHCP_H

#define DHCP_SERVER_PORT    (67)
#define DHCP_CLIENT_PORT    (68)

#define DHCP_UDP_OVERHEAD   (14 + 20 + 8)

#define DHCP_SNAME_LEN      64
#define DHCP_FILE_LEN       128
#define DHCP_FIXED_NON_UDP  236
#define DHCP_FIXED_LEN      (DHCP_FIXED_NON_UDP + DHCP_UDP_OVERHEAD)
#define DHCP_MTU_MAX        1500
#define DHCP_OPTION_LEN     (DHCP_MTU_MAX - DHCP_FIXED_LEN)

struct dhcp_packet {
    u_int8_t    op;
    u_int16_t   htype;
    u_int8_t    hlen;
    u_int8_t    hops;
    u_int32_t   xid;
    u_int16_t   secs;
    u_int16_t   flags;
    struct      in_addr ciaddr;
    struct      in_addr yiaddr;
    struct      in_addr siaddr;
    struct      in_addr giaddr;
    u_int8_t    chaddr[16];
    char        sname[DHCP_SNAME_LEN];
    char        file[DHCP_FILE_LEN];
    u_int8_t    options[DHCP_OPTION_LEN];
    
};

#endif //NETWORKEDU_DHCP_H
