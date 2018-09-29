//
// Created by hhoshino on 18/09/28.
//

#include <stdio.h>
#include <ctype.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <errno.h>
#include <signal.h>
#include <time.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <sys/param.h>
#include <sys/types.h>
#include <sys/ioctl.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netpacket/packet.h>
#include <net/ethernet.h>
#include <netinet/if_ether.h>
#include <netinet/in.h>
#include <netdb.h>
#include <sys/wait.h>
#include <netinet/in.h>
#include <netinet/ip.h>
#include <netinet/ip6.h>
#include "sock.h"
#include "ether.h"
#include "param.h"
#include "Common.h"

extern PARAM Param;

static char *ParamFname = NULL;

const int BUFFER_SIZE   = 1024;

int SetDefaultParam() {

    Param.MTU   = DEFAULT_MTU;
    Param.IpTTL = DEFAULT_IP_TTL;
    return PROCESS_RESULT_SUCCESS;

}

int ReadParam(char *fname) {

    FILE *fp;
    char buf[BUFFER_SIZE];
    char *ptr, *saveptr;

    ParamFname = fname;

    if ((fp=fopen(fname, "r")) == NULL) {
        printf("%s cannot read\n", fname);
        return PROCESS_RESULT_ERROR;
    }

    while(1) {

        fgets(buf,sizeof(buf), fp);

        if (feof(fp)) {
            break;
        }

        ptr = strtok_r(buf ,"=", &saveptr);

        if (ptr != NULL) {

            if (strcmp(ptr, "IP-TTL")==0) {
                if ((ptr=strtok_r(NULL, "\r\n", &saveptr)) != NULL) {
                    Param.IpTTL = atoi(ptr);
                }
            }

        } else if(strcmp(ptr,"MTU") == 0) {

            if ((ptr=strtok_r(NULL, "\r\n", &saveptr)) != NULL) {
                Param.MTU = atoi(ptr);

                if (Param.MTU > ETHERMTU) {
                    printf("ReadParam:MTU(%d <= ETHERMTU(%d)\n",Param.MTU, ETHERMTU);
                    Param.MTU = ETHERMTU;
                }
            }

        } else if (strcmp(ptr,"gateway") == 0) {

            if ((ptr=strtok_r(NULL, "\r\n", &saveptr)) != NULL) {
                Param.gateway.s_addr = inet_addr(ptr);
            }

        } else if (strcmp(ptr,"device") == 0) {

            if((ptr == strtok_r(NULL,"\r\n",&saveptr)) != NULL) {
                Param.device = strdup(ptr);
            }

        } else if (strcmp(ptr,"vmac") == 0) {

            if ((ptr=strtok_r(NULL,"\r\n",&saveptr)) != NULL) {
                my_ether_aton(ptr,Param.vmac);
            }

        } else if (strcmp(ptr,"vip") == 0) {

            if ((ptr=strtok_r(NULL,"\r\n",&saveptr)) != NULL) {
                Param.vip.s_addr = inet_addr(ptr);
            }

        } else if ( strcmp(ptr,"vmask") == 0) {

            if ((ptr=strtok_r(NULL,"\r\n",&saveptr)) != NULL) {
                Param.vmask.s_addr = inet_addr(ptr);
            }

        }

    }

    fclose(fp);

    return PROCESS_RESULT_SUCCESS;

}

int isTargetIPAddr(struct in_addr *addr) {

    if (Param.vip.s_addr == addr->s_addr) {
        return PROCESS_RESULT_ERROR;
    }

    return PROCESS_RESULT_SUCCESS;

}

int isSameSubnet(struct in_addr *addr) {

    if ((addr->s_addr & Param.vmask.s_addr) ==
            (Param.vip.s_addr & Param.vmask.s_addr)) {
        return PROCESS_RESULT_ERROR;
    }

    return PROCESS_RESULT_SUCCESS;

}