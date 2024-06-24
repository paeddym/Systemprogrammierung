#ifndef BROADCASTAGENT_H
#define BROADCASTAGENT_H

#include "network.h"
#include "user.h"

int broadcastAgentInit(void);
void broadcastAgentCleanup(void);
int receiveMessage(int fd, Message *buffer);
void sendMessage(User *myUser, Message *buffer);
#endif
