#ifndef USER_H
#define USER_H

#include "network.h"
#include <pthread.h>

typedef struct User
{
	struct User *prev;
	struct User *next;
	pthread_t thread;
	int sock;			
	char name[32];
} User;

User *allocateSpaceForUser();

User *addUser();
void iterateOverSockets(int (*func)(int, const Message *), User *self, void *buffer);

void cleanUpOfUser(User *deleteUser);
void removeUser(User *myUser);

int initMutex();
void lockUser();
void unLockUser();

User *getUserTroughName(const char *name);

User *getFirstUser();

#endif
