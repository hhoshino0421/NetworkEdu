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

int print_dhcp(struct dhcp_packet *pa,int size) {
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
                    break;
                }
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

                        break;
                    }


                }
            }

            if (end) {
                break;
            }

        }
    }


    return 0;

}

u_int8_t *dhcp_set_option(u_int8_t *ptr,int tag,int size,u_int8_t *buf) {
    
}


