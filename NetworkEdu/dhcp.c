//
// Created by hhoshino on 18/11/11.
//

#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <time.h>
#include <sys/ioctl.h>
#include <netpacket/packet.h>
#include <netinet/if_ether.h>
#include <netinet/ip.h>
#include <netinet/udp.h>
#include <linux/if.h>
#include <arpa/inet.h>
#include "sock.h"
#include "ether.h"
#include "arp.h"
#include "ip.h"
#include "udp.h"
#include "dhcp.h"
#include "param.h"

extern PARAM        Param;
extern u_int8_t     BcastMac[6];

int print_dhcp(struct dhcp_packet *pa, int size) {

    int             i;
    char            cookie[4];
    u_int8_t        *ptr;
    struct in_addr  addr;
    u_int32_t       l;
    u_int16_t       s;
    int             end, n;
    char            buf[512], buf1[80];

    printf("dhcp--------------------------------------------------------\n");

    printf("op=%d:", pa->op);

    if(pa->op == BOOTREQUEST) {
        printf("BOOTREQUEST\n");
    } else if (pa->op == BOOTREPLY) {
        printf("BOOTREPLY\n");
    } else {
        printf("UNDEFINE\n");
        return -1;
    }

    printf("htype=%d:", pa->htype);

    if (pa->htype==HTYPE_IEEE802) {
        printf("HTYPE_IEEE802\n");
    } else {
        printf("UNDEFINE\n");
        return -1;
    }

    printf("hlen%d\n", pa->hlen);
    printf("hops=%d\n", pa->hops);
    printf("xid=%u\n", pa->xid);
    printf("secs=%d\n", pa->secs);
    printf("flags=%x\n", pa->flags);
    printf("ciaddr=%s\n", inet_ntop(AF_INET, &pa->ciaddr, buf1, sizeof(buf1)));
    printf("yiaddr=%s\n", inet_ntop(AF_INET, &pa->yiaddr, buf1, sizeof(buf1)));
    printf("siaddr=%s\n", inet_ntop(AF_INET, &pa->siaddr, buf1, sizeof(buf1)));
    printf("giaddr=%s\n", inet_ntop(AF_INET, &pa->giaddr, buf1, sizeof(buf1)));
    printf("sname=%s\n", pa->sname);
    printf("file=%s\n", pa->file);

    printf("optios\n");

    ptr=pa->options;

    memcpy(cookie, ptr, 4);
    ptr += 4;

    if(memcmp(cookie, DHCP_OPTIONS_COOKIE, 4) != 0) {
        printf("options:cookie:error\n");
        return -1;
    }

    end = 0;

    while(ptr < (u_int8_t *)pa + size) {

        switch(*ptr) {
            case 0: {
                printf("0:pad\n");
                ptr++;
                break;
            }

            case 1: {
                printf("1:subnet mask:");
                ptr++;
                n = *ptr;
                ptr++;
                printf("%d:",n);
                memcpy(&addr, ptr, 4);
                ptr += 4;
                printf("%s\n", inet_ntop(AF_INET, &addr, buf1, sizeof(buf1)));
                break;
            }

            /* */
            case 61: {
                printf("61:client-identifer:");
                ptr++;
                n = *ptr;
                ptr++;
                printf("%d", n);
                for (i = 0; i < n; i++) {
                    if (i != 0) {
                        printf(":");
                    }
                    printf("%02X",(*ptr)&0xFF);
                    ptr++;
                    printf("\n");
                    //break;
                }
                break;
            }

            default: {

                if(*ptr >= 128 && *ptr <= 254) {
                    printf("%d:reserved fields:", *ptr);
                    ptr++;
                    n = *ptr;
                    ptr++;
                    printf("%d:",n);

                    for (i = 0; i < n; i++) {
                        if (i != 0) {
                            printf(":");
                        }

                        printf("%02X", (*ptr)&0xFF);
                        ptr++;
                        printf("\n");
                    }

                } else {
                    printf("%d:undefined:", *ptr);
                    ptr++;
                    n = *ptr;
                    ptr++;
                    printf("%d:", n);

                    for (i = 0; i < n; i++) {
                        if (i != 0) {
                            printf(":");
                            printf("%02X",(*ptr)&0xFF) ;
                            ptr++;
                            printf("\n");

                        }

                        //break;
                    }

                    break;

                }
            }

            if (end) {
                break;
            }

        }
    }


    return 0;

}

u_int8_t *dhcp_set_option(u_int8_t *ptr, int tag, int size, u_int8_t *buf) {

    ptr++;

    if (size > 255){
        size = 255;
    }

    *ptr = (u_int8_t)size;
    ptr++;
    memcpy(ptr, buf, size);
    ptr += size;

    return ptr;

}


int dhcp_get_option(struct dhcp_packet *pa, int size, int opno, void *val) {

    u_int8_t    cookie[4];
    u_int8_t    *ptr;
    int         end, n;

    memcpy(cookie, ptr, 4);
    ptr += 4;

    if (memcmp(cookie, DHCP_OPTIONS_COOKIE, 4) != 0) {
        printf("analize_packet:options:cookie:error\n");
        return -1;
    }

    end = 0;

    while (ptr < (u_int8_t *)pa + size) {

        if (*ptr == 0) {
            ptr++;
        } else if (*ptr == 255) {
            end=1;
        } else if (*ptr == opno) {
            ptr++;
            n = *ptr;
            ptr++;
            memcpy(val, ptr, n);
            ptr += n;
            end = 1;
        } else {
            ptr++;
            n = *ptr;
            ptr++;
            ptr += n;
        }

        if(end) {
            break;
        }

    }

    return 0;
}


int MakeDhcpRequest(struct dhcp_packet *pa, u_int8_t mtype, struct in_addr *ciaddr, struct in_addr *rep_ip, struct in_addr *server) {

    u_int8_t    *ptr;
    u_int8_t    buf[512];
    int         size;
    u_int32_t   l;

    memset(pa,0,sizeof(struct dhcp_packet));
    pa->op = BOOTREQUEST;
    pa->htype = HTYPE_ETHER;
    pa->hlen = 6;
    pa->hops = 0;
    pa->xid = htons(getpid()&0xFFFF);
    pa->secs = 0;
    pa->flags = htons(0x8000);

    if (ciaddr ==  NULL) {
        pa->ciaddr.s_addr = 0;
    } else {
        pa->ciaddr.s_addr = ciaddr->s_addr;
    }

    pa->yiaddr.s_addr = 0;
    pa->siaddr.s_addr = 0;
    pa->giaddr.s_addr = 0;
    memcpy(pa->chaddr, Param.vmac, 6);
    strcpy(pa->sname, "");
    strcpy(pa->file, "");

    ptr = pa->options;
    memcpy(ptr, DHCP_OPTIONS_COOKIE, 4);
    ptr += 4;

    buf[0] = mtype;
    ptr = dhcp_set_option(ptr, 53, 1, buf);

    l=htons(Param.DhcpRequestLeaseTime);
    ptr = dhcp_set_option(ptr, 51, 4, (u_int8_t *)&l);

    if (req_ip != NULL) {
        ptr = dhcp_set_option(ptr, 50, 4, (u_int8_t *)&req_ip->s_addr);
    }

    if (server!=NULL) {
        ptr=dhcp_set_option(ptr,54,4,(u_int8_t *)&server->s_addr);
    }

    buf[0] = 1;
    buf[1] = 3;

    ptr = dhcp_set_option(ptr, 55, 2, buf);
    ptr = dhcp_set_option(ptr, 255, 0, NULL);

    size = ptr - (u_int8_t *)pa;

    return size;

}

int DhcpSendDiscover(int soc) {

    int                     size;
    struct  dhcp_packet     pa;
    struct  in_addr         saddr, daddr;

    saddr.s_addr     = 0;
    inet_aton("255.255.255.255", &daddr);
    size=MakeDhcpRequest(&pa, DHCPDISCOVER, NULL, NULL, NULL);

    printf("--- DHCP ---{\n");

    UdpSendLink(soc, Param.vmac, BcastMac, &saddr, &daddr
            , DHCP_CLIENT_PORT, DHCP_SERVER_PORT, 1, (u_int8_t)&pa, size);

    print_dhcp(&pa, size);

    printf("}\n");

    return 0;

}


int DhcpSendRequest(int soc, struct in_addr *yiaddr, struct in_addr *server) {

    int                 size;
    struct  dhcp_packet pa;
    struct  in_addr     saddr, daddr;

    saddr.s_addr = 0;


}

