#ifndef ONE_NET_H
#define ONE_NET_H

#include "Hal.h"

void OneNetDataReport(char *data);
void OneNetClose(void);
int OneNetStartConnect(void);
bool OneNetConnected(void);
void OneNetDataReport(char *data);

void OneNetInitialize(void);
void OneNetPoll(void);
#endif

