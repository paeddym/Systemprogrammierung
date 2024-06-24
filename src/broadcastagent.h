#ifndef BROADCASTAGENT_H
#define BROADCASTAGENT_H

#include "network.h"
#include "user.h"

int broadcastAgentInit(void);
void broadcastAgentCleanup(void);
int receive(int fd, Message *buffer);
void send(User *myUser, Message *buffer);
#endif
