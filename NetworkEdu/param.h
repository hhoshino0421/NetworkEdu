//
// Created by hhoshino on 18/09/28.
//

#include <stdint.h>

#ifndef NETWORKEDU_PARAM_H
#define NETWORKEDU_PARAM_H

/* 定数宣言 */
#define DEFAULT_MTU         (ETHERMTU)
#define DEFAULT_IP_TTL      (64)
#define DEFAULT_PING_SIZE   (64)

#define DUMMY_WAIT_MS       (100)
#define RETRY_COUNT         (3)

/* 構造体定義 */
typedef struct {
    char            *device;
    u_int8_t        mymac[6];
    struct in_addr  myip;
    u_int8_t        vmac[6];
    struct in_addr  vip;
    struct in_addr  vmask;
    int             IpTTL;
    int             MTU;
    struct in_addr  gateway;
} PARAM;

/* 関数プロトタイプ宣言 */
int SetDefaultParam();
int ReadParam(char *fname);
int isTargetIPAddr(struct in_addr *addr);
int isSameSubnet(struct in_addr *addr);



#endif //NETWORKEDU_PARAM_H
