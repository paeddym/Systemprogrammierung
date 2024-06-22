#ifndef USER_H
#define USER_H

#include <pthread.h>
#include "network.h"

typedef struct User
{
	struct User *prev;
	struct User *next;
	pthread_t thread;	//thread ID of the client thread
	int sock;		//socket for client
	char name[32];
} User;

//TODO: Add prototypes for functions that fulfill the following tasks:
// * Add a new user to the list and start client thread
User *allocateSpace();
User *addUser();

// * Iterate over the complete list (to send messages to all users)
void iterateList(User *self);


// * Remove a user from the list
void cleanUp(User *deleteUser);
void removeUser(User *myUser);

//CAUTION: You will need proper locking!
int initMutex();
void lockUser();
void unLockUser();

#endif
