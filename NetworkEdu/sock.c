//
// Created by hhoshino on 18/09/28.
//

#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include <sys/ioctl.h>
#include <netpacket/packet.h>
#include <netinet/ip.h>
#include <netinet/ip_icmp.h>
#include <netinet/if_ether.h>
#include <linux/if.h>
#include <arpa/inet.h>
#include "sock.h"
#include "param.h"
#include "Common.h"

u_int16_t checksum(u_int8_t *data, int len) {

    u_int32_t   sum;
    u_int16_t   *ptr;
    int         c;

    sum = 0;
    ptr = (uint16_t*)data;

    for (c = len; c > 1; c -= 2) {

        sum += (*ptr);

        if (sum & 0x80000000) {
            sum = (sum & 0xFFFF) + (sum >> 16);
        }

        ptr++;

    }

    if (c == 1) {

        uint16_t    val;
        val = 0;
        memcpy(&val, ptr, sizeof(u_int8_t));
        sum += val;

    }

    while(sum >> 16) {
        sum = (sum & 0xFFFF) + (sum >> 16);
    }

    return (~sum);

}

u_int16_t checksum2(u_int8_t *data1, int len1, u_int8_t *data2, int len2) {

    u_int32_t   sum;
    u_int16_t   *ptr;
    int         c;

    sum = 0;
    ptr = (u_int16_t *)data1;

    for (c = len1; c > 1; c -= 2) {
        sum += (*ptr);
        if (sum & 0x80000000) {
            sum = (sum & 0xFFFF) + (sum >> 16);
        }

        ptr++;
    }

    if (c == 1) {

        u_int16_t   val;
        val = ((*ptr) << 8) + (*data2);
        sum += val;

        if (sum & 0X8000000) {
            sum = (sum&0xFFFF) + (sum>>16);
        }

        ptr=(u_int16_t *)(data2 + 1);
        len2--;

    } else {

        ptr=(u_int16_t *)data2;

    }

    for (c=len2;c>1;c-=2){

        sum += (*ptr);

        if (sum & 0x80000000) {
            sum = (sum & 0xFFFF) + (sum >> 16);
        }

        ptr++;
    }

    if (c == 1) {

        u_int16_t   val;
        val = 0;
        memcpy(&val, ptr, sizeof(u_int8_t));
        sum += val;

    }

    while(sum >> 16) {

        sum = (sum & 0xFFFF) + (sum >> 16);

    }

    return (~sum);

}

int GetMacAddress(char *device, u_int8_t *hwaddr) {

    struct ifreq    ifreq;
    int             soc;
    u_int8_t        *p;

    if ((soc = socket(AF_INET, SOCK_DGRAM, 0)) < 0) {

        perror("GetMacAdress():socket");
        return PROCESS_RESULT_ERROR;

    }

    strncpy(ifreq.ifr_name, device, sizeof(ifreq.ifr_name) - 1);

    if (ioctl(soc, SIOCGIFADDR, &ifreq) == -1) {

        perror("GetMacAdress():hwaddr");
        close(soc);
        return PROCESS_RESULT_ERROR;

    } else {

        p = (u_int8_t *)&ifreq.ifr_hwaddr.sa_data;
        memcpy(hwaddr, p, 6);
        close(soc);

        return 1;

    }

}

int DummyWait(int ms) {

    struct timespec     ts;

    ts.tv_sec   = 0;
    ts.tv_nsec  = ms * 1000 * 1000;

    nanosleep(&ts,NULL);

    return PROCESS_RESULT_SUCCESS;

}