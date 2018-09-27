#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
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
#include "sock.h"
#include "ether.h"
#include "arp.h"
#include "ip.h"
#include "icmp.h"
#include "param.h"
#include "cmd.h"


/* 終了フラグ */
int EndFlag = 0;
/* 送受信するディスクリプタ */
int DeviceSoc;
/* 設定を保持 */
PARAM   Param;



void *MyEthThread(void *arg) {
    int             nready;
    struct pollfd   targets[1];
    u_int8_t        buf[2048];
    int             len;

    targets[0].fd       = DeviceSoc;
    targets[0].events   = POLLIN | POLLERR;

    while(EndFlag == 0) {

        switch((nready=poll(targets,1,1000))) {
            case -1: {
                if (errno != EINTR) {
                    perror("poll");
                }
                break;
            }
            case 0: {
                break;
            }
            default: {
                if (targets[0].revents&(POLLIN|POLLERR)) {
                    if((len=read(DeviceSoc, buf, sizeof(buf))) <= 0){
                        perror("read");
                    } else {
                        EtherRecv(DeviceSoc, buf, len);
                    }
                }
                break;
            }

        }
    }

    return(NULL);

}


void *StdInThread(void *arg) {

    int                 nready;
    struct  pollfd      targets[1];
    char                buf[2048];

    targets[0].fd       = fileno(stdin);
    targets[0].events   = POLLIN | POLLERR;

    while(EndFlag == 0) {
        switch((nready=poll(targets,1,1000))) {
            case -1:{
                if(errno!=EINTR) {
                    perror("poll");
                }
                break;
            }
            case 0: {
                break;
            }
            default: {
                if(targets[0].revents&(POLLIN|POLLERR)) {
                    fgets(buf,sizeof(buf),stdin);
                    DoCmd(buf);
                }
                break;
            }
        }
    }

    return(NULL);

}

void sig_term(int sig) {
    EndFlag = 1;
}

int ending() {
    struct ifreq        if_req;

    printf("ending\n");

    if (DeviceSoc != -1) {
        
        strcpy(if_req.ifr_name, Param.device);
        if (ioctl(DeviceSoc,SIOCGIFFLAGS,&if_req)<0) {
            perror("ioctl");
        }

        if_req.ifr_flags = if_req.ifr_flags&~IFF_PROMISC;
        if (ioctl(DeviceSoc,SIOCGIFFLAGS,&if_req) < 0) {
            perror("ioctl");
        }

        close(DeviceSoc);
        DeviceSoc = -1;

    }

    return 0;
}

int main() {

    return 0;
}