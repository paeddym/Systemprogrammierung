#ifndef CLIENTTHREAD_H
#define CLIENTTHREAD_H
#include "user.h"
#include "network.h"

enum COMMANDS {
    kickClientCommandCode = 0,
    pauseChatCommandCode = 1,
    resumeChatCommandCode = 2,
    invalidCommandCode = 3
};

void *clientthread(void *arg);
int getHeaderLength(Message *buffer);
int validateClientVersionAndName(const char *clientName, uint8_t clientVersion);
void extractUserName(const char *text, char *userName);
void handleUserRemoval(User *userToRemove, int urmCode);
int handleLogin(User *self, char *clientName, Message *lrq);

#endif
