#ifndef BROADCASTAGENT_H
#define BROADCASTAGENT_H

#include "network.h"
#include "user.h"

int broadcastAgentInit(void);
void broadcastAgentCleanup(void);
int receive(int fd, Message *buffer);
void broadcastMessage(Message *buffer, User *skipUser);
void send(User *myUser, Message *buffer);
void sendToMessageQueue(Message *buffer, User *user);

#endif
