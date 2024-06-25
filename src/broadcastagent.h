#ifndef BROADCASTAGENT_H
#define BROADCASTAGENT_H

#include "network.h"
#include "user.h"

enum SEMAPHORE {
    pshared = 0,
    paused = 0U,
    running = 1U
};

int receiveMessage(int fd, Message *buffer);
void broadcastMessage(Message *buffer, User *skipUser);
void sendMessage(User *myUser, Message *buffer);
int broadcastAgentInit(void);
void broadcastAgentCleanup(void);
void sendToMessageQueue(Message *buffer, User *user);
void pauseChat();
void resumeChat();
uint8_t getChatStatus();

#endif


