#ifndef CLIENTTHREAD_H
#define CLIENTTHREAD_H
#include "user.h"
#include "network.h"

void *clientthread(void *arg);

int getHeaderLength(Message *buffer);
int checkLoginRequest(const char *clientName, uint8_t version);
void handleUserRemoved(User *user, int code);
int handleLogin(User *self, char *name, Message *loginRequest);

#endif
