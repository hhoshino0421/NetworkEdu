//
// Created by hhoshino on 18/10/04.
//

#ifndef NETWORKEDU_CMD_H
#define NETWORKEDU_CMD_H

/* 関数プロトタイプ宣言 */
int DoCmdArp(char **cmdline);
int DoCmdPing(char **cmdline);
int DoCmdIfconfig(char **cmdline);
int DoCmdNetstat(char **cmdline);
int DoCmdEnd(char **cmdline);
int DoCmd(char *cmd);

int DoCmdUpd(char **cmdline);

#endif //NETWORKEDU_CMD_H
