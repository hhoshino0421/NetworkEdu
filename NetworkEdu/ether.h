//
// Created by hhoshino on 18/09/30.
//

#include <unistd.h>

#ifndef NETWORKEDU_ETHER_H
#define NETWORKEDU_ETHER_H

/* 関数プロトタイプ宣言 */
char *my_ether_ntoa_r(u_int8_t *hwaddr, char *buf);
int my_ether_aton(char *str, u_int8_t *mac);
int print_hex(u_int8_t *data, int size);
void print_ether_header(struct ether_header *eh);
int EtherSend(int soc, u_int8_t smac[6], u_int8_t dmac[6], u_int16_t type, u_int8_t *data, int len);
int EtherRecv(int soc, u_int8_t *in_ptr, int in_len);


#endif //NETWORKEDU_ETHER_H
