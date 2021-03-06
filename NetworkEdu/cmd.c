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

#include <netinet/udp.h>
#include "upd.h"

extern int      DeviceSoc;

extern PARAM    Param;

int DoCmdArp(char **cmdline) {

    char *ptr;

    if ((ptr = strtok_r(NULL, " ", cmdline)) == NULL) {
        printf("DoCmdArp:no arg\n");
        return -1;
    }

    if ((ptr = strtok_r(NULL, " ", cmdline)) == NULL) {
        printf("DoCmdArp:no arg\n");
        return -1;
    }

    if (strcmp(ptr, "-a") == 0) {

        ArpShowTable();
        //printf("apr -a not coding.\n");
        return 0;

    } else if (strcmp(ptr, "-d") == 0) {

        if ((ptr = strtok_r(NULL, " ", cmdline)) == NULL){
            printf("DoCmdArp:-d no arg\n");
            return -1;
        }

        struct in_addr  addr;
        inet_aton(ptr, &addr);

        if (ArpDelTable(&addr)) {
            printf("deleted\n");
        } else {
            printf("not exists\n");
        }

        return 0;

    } else {

        printf("DocmdArp:[%s] unknown \n", ptr);
        return -1;
    }

}


int DoCmdPing(char **cmdline) {

    char                *ptr;
    struct  in_addr     daddr;
    int                 size;

    if ((ptr = strtok_r(NULL, " ", cmdline)) == NULL) {
        printf("DoCmdPing:no arg\n");
        return -1;
    }

    if ((ptr = strtok_r(NULL, " ", cmdline)) == NULL) {
        printf("DoCmdPing:no arg\n");
        return -1;
    }

    inet_aton(ptr, &daddr);

    if ((ptr = strtok_r(NULL, " ", cmdline)) == NULL) {
        size = DEFAULT_PING_SIZE;
    } else {
        size = atoi(ptr);
    }

    PingSend(DeviceSoc,&daddr,size);

    return 0;

}


int DoCmdIfconfig(char **cmdline) {

    char    buf1[80];

    printf("device=%s\n", Param.device);
    printf("vmac=%s\n", my_ether_ntoa_r(Param.vmac, buf1));
    printf("vip=%s\n", inet_ntop(AF_INET, &Param.vip, buf1, sizeof(buf1)));
    printf("vmac=%s\n", inet_ntop(AF_INET, &Param.vmask, buf1, sizeof(buf1)));
    printf("gateway=%s\n", inet_ntop(AF_INET, &Param.gateway, buf1, sizeof(buf1)));

    if (Param.DhcpStartTime==0) {
        printf("Static\n");
    } else {
        printf("DHCP request lease time=%d\n", Param.DhcpRequestLeaseTime);
        printf("DHCP server=%s\n", inet_ntop(AF_INET, &Param.DhcpServer, buf1, sizeof(buf1)));
        printf("DHCP start time:%s", ctime(&Param.DhcpStartTime));
        printf("DHCP lease time:%s", Param.DhcpLeaseTime);
    }

    printf("IpTTL=%d,MTU=%d\n", Param.IpTTL, Param.MTU);

    return 0;

}


int DoCmdEnd(char **cmdline) {

    kill(getpid(), SIGTERM);

    return 0;

}


int DoCmdNetstat(char **cmdline) {

    printf("----------------------------------------------\n");
    printf("proto:no:port:data\n");

    UdpShowTable();

    return 0;

}


int DoCmdUdp(char **cmdline) {

    char        *ptr;
    u_int16_t   port;
    int         no, ret;

    if ((ptr=strtok_r(NULL, "\n", cmdline)) == NULL) {
        printf("DoCmdUdp:no arg\n");
        return -1;
    }

    if (strcmp(ptr, "open") == 0) {

        if ((ptr = strtok_r(NULL, "\n", cmdline)) == NULL) {
            no = UpdSocket(0);
        } else {
            port = atoi(ptr);
            no = UpdSocket(port);
        }

        printf("DoCmdUpd:no=%d\n", no);

    } else if (strcmp(ptr, "close") == 0) {

        if ((ptr = strtok_r(NULL, "\n", cmdline)) == NULL) {
            printf("DoCmdUpd:close:no arg\n");
            return -1;
        }

        port = atoi(ptr);
        ret = UpdSocketClose(port);
        printf("DoCmdUpd:ret=%d",ret);

    } else if (strcmp(ptr,"send") == 0) {

        char            *p_addr, *p_port;
        struct in_addr  daddr;
        u_int16_t       sport, dport;

        if((ptr = strtok_r(NULL, "\n", cmdline)) == NULL) {
            printf("DoCmdUpd:send no arg\n");
            return -1;
        }

        sport = atoi(ptr);

        if ((p_addr = strtok_r(NULL, ":\n", cmdline)) == NULL) {
            printf("DoCmdUpd: send %u no arg\n",sport);
            return -1;
        }

        if ((p_port = strtok_r(NULL, "\n", cmdline)) == NULL) {
            printf("DoCmdUpd:send %u %s:no arg\n", sport, p_addr);
            return -1;
        }

        inet_aton(p_addr, &daddr);
        dport = atoi(p_port);

        if ((ptr = strtok_r(NULL, "\n", cmdline)) == NULL) {
            printf("DoCmdUpd:;send%u %s:%d no arg\n", sport, p_addr, dport);
            return -1;
        }

        MakeString(ptr);

        UpdSend(DeviceSoc, &Param.vip, &daddr, sport, dport, 0, (u_int8_t *)ptr, strlen(ptr));

    } else {

        printf("DoCmdUpd:[%s] unknown\n", ptr);
        return -1;

    }

    return 0;

}


int DoCmd(char *cmd) {

    char    *ptr, *saveptr;

    if ((ptr = strtok_r(cmd, "\n", &saveptr)) == NULL) {

        printf("DoCmd:no cmd\n");
        printf("-------------------------------------------------------------------------\n");
        printf("arp -a : show arp table\n");
        printf("arp -d addr : del arp table\n");
        printf("ping addr [size] : send ping\n");
        printf("ifconfig : show interface configuration\n");

        printf("netstat : show active ports\n");
        printf("udp open port : open udp-recv port\n");
        printf("udp close port : close udp-recv port\n");
        printf("udp send sport daddr:dport data : send udp\n");

        printf("end : end program\n");
        printf("-------------------------------------------------------------------------\n");

        return -1;

    }


    if ((strncmp(ptr, "arp -a", 6) == 0 || strncmp(ptr,"arp -d", 6) == 0)) {

        DoCmdArp(&ptr);
        return 0;

    } else if (strncmp(ptr,"ping", 4) == 0) {

        DoCmdPing(&ptr);
        return 0;

    } else if (strcmp(ptr,"ifconfig") == 0) {

        DoCmdIfconfig(&saveptr);
        return 0;

    } else if (strcmp(ptr,"netstat") == 0) {

        DoCmdNetstat(&saveptr);

    } else if (strcmp(ptr,"udp") == 0) {

        DoCmdUdp(&ptr);

    } else if (strcmp(ptr,"end") == 0) {

        DoCmdEnd(&saveptr);
        return 0;

    } else {

        printf("DoCmd:unknown cmd : %s", ptr);
        return -1;
    }

}


int MakeString(char *data) {

    char *tmp = strdup(data);
    char *wp, *rp;

    for (wp = tmp, rp=data; *rp != '\0'; rp++) {

        if (*rp == '\\' && *(rp + 1) != '\0') {
            rp++;

            switch (*rp) {

                case 'n': {
                    *wp = '\n';
                    wp++;
                    break;
                }

                case 'r': {
                    *wp = '\r';
                    wp++;
                    break;
                }

                case 't': {
                    *wp = '\t';
                    wp++;
                    break;
                }

                case '\\': {
                    *wp = '\\';
                    wp++;
                    break;
                }
                default: {
                    *wp = '\\';
                    wp++;
                    break;
                }

            }

        } else {

            *wp = *rp;
            wp++;

        }

    }

    *wp = '\0';
    strcpy(data, tmp);
    free(tmp);

    return 0;

}

