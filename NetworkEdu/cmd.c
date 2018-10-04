//
// Created by hhoshino on 18/10/04.
//

#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <errno.h>
#include <poll.h>
#include <sys/ioctl.h>
#include <netinet/ip_icmp.h>
#include <netinet/if_ether.h>
#include <linux/if.h>
#include <arpa/inet.h>
#include <sys/wait.h>
#include <pthread.h>
#include <stdlib.h>     //Add Hoshino Hitoshi
#include "sock.h"
#include "ether.h"
#include "arp.h"
#include "icmp.h"
#include "param.h"
#include "cmd.h"
#include "Common.h"

extern int      DeviceSoc;

extern PARAM    Param;

int DoCmdArp(char **cmdline) {

    char *ptr;

    if ((ptr=strtok_r(NULL,"\r\n", cmdline)) == NULL) {
        printf("DoCmdArp:no arg\n");
        return PROCESS_RESULT_ERROR;
    }

    if (strcmp(ptr,"-a") == 0) {

        ArpShowTable();
        return PROCESS_RESULT_SUCCESS;

    } else if (strcmp(ptr,"-d") == 0) {

        if ((ptr = strtok_r(NULL, "\r\n", cmdline)) == NULL){
            printf("DoCmdArp:-d no arg\n");
            return PROCESS_RESULT_ERROR;
        }

        struct in_addr  addr;
        inet_aton(ptr, &addr);

        if (ArpDelTable(&addr)) {
            printf("deleted\n");
        } else {
            printf("not exists\n");
        }

        return PROCESS_RESULT_SUCCESS;

    } else {

        printf("DocmdArp:[%s] unknown \n", ptr);
        return PROCESS_RESULT_ERROR;
    }

}


int DoCmdPing(char **cmdline) {

    char                *ptr;
    struct  in_addr     daddr;
    int                 size;

    if ((ptr == strtok_r(NULL, "\r\n", cmdline)) == NULL) {
        printf("DoCmdPing:no arg\n");
        return PROCESS_RESULT_ERROR;
    }

    inet_aton(ptr, &daddr);

    if ((ptr = strtok_r(NULL, "\r\n", cmdline)) == NULL) {
        size = DEFAULT_PING_SIZE;
    } else {
        size = atoi(ptr);
    }

    PingSend(DeviceSoc,&daddr,size);

    return PROCESS_RESULT_SUCCESS;

}


int DoCmdIfconfig(char **cmdline) {

    char    buf1[80];

    printf("device=%s\n", Param.device);
    printf("vmac=%s\n", my_ether_ntoa_r(Param.vmac, buf1));
    printf("vip=%s\n", inet_ntop(AF_INET, &Param.vip, buf1, sizeof(buf1)));
    printf("vmac=%s\n", inet_ntop(AF_INET, &Param.vmask, buf1, sizeof(buf1)));
    printf("gateway=%s\n", inet_ntop(AF_INET, &Param.gateway, buf1, sizeof(buf1)));
    printf("IpTTL=%d,MTU=%d\n", Param.IpTTL, Param.MTU);

    return PROCESS_RESULT_SUCCESS;

}


int DoCmdEnd(char **cmdline) {

    kill(getpid(), SIGTERM);

    return PROCESS_RESULT_SUCCESS;

}


int DoCmd(char *cmd) {

    char    *ptr, *saveptr;

    if ((ptr = strtok_r(cmd, "\r\n", &saveptr)) == NULL) {

        printf("DoCmd:no cmd\n");
        printf("-------------------------------------------------------------------------\n");
        printf("arp -a : show arp table\n");
        printf("arp -d addr : del arp table\n");
        printf("ping addr [size] : send ping\n");
        printf("ifconfig : show interface configuration\n");
        printf("end : end program\n");
        printf("-------------------------------------------------------------------------\n");

        return PROCESS_RESULT_ERROR;

    }

    if (strcmp(ptr,"arp") == 0) {

        DoCmdArp(&saveptr);
        return PROCESS_RESULT_SUCCESS;

    } else if (strcmp(ptr,"ping") == 0) {

        DoCmdPing(&saveptr);
        return PROCESS_RESULT_SUCCESS;

    } else if (strcmp(ptr,"ifconfig") == 0) {

        DoCmdIfconfig(&saveptr);
        return PROCESS_RESULT_SUCCESS;

    } else if (strcmp(ptr,"end") == 0) {

        DoCmdEnd(&saveptr);
        return PROCESS_RESULT_SUCCESS;

    } else {

        printf("DoCmd:unknown cmd : %s", ptr);
        return PROCESS_RESULT_ERROR;
    }

}
