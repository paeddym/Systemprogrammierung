#include <pthread.h>
#include "user.h"
#include "clientthread.h"
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h> 
#include "network.h"

static pthread_mutex_t userLock = PTHREAD_MUTEX_INITIALIZER;
static User *userFront = NULL;
static User *userBack = NULL;

User *allocateSpace()
{
    User *newUser = (User *)malloc(sizeof(User));
    if(newUser == NULL) {
        errno = ENOMEM;
        perror("Failed to allocate memory!");
        return NULL;
    }
    return newUser;
}

User *addUser(User *newUser)
{
    if(userFront == NULL) {
        newUser->prev = NULL;
        newUser->next = NULL;
        userFront = newUser;
        userBack = newUser;
    } else {
        userBack->next = newUser;
        newUser->prev = userBack;
        newUser->next = NULL;
        userBack = newUser;
    }
    return newUser;
}

void iterateList(void (* func)(User *, char *), User *myUser, char *buf) {
    User *currentUser = userFront;
    User *next;
    lockUser();
    while(currentUser != NULL)
    {
        next = currentUser->next;
        if(currentUser != myUser)
        {
            func(currentUser, buf);
        }
        currentUser = next;
    }
    unlockUser();
}

static bool onlyUser(User *currentUser)
{
    if(currentUser == userFront && currentUser == userBack) {
        return true;
    } else {
        return false;
    }
}
static bool firstUser(User *currentUser)
{
    if(currentUser == userFront && currentUser != userBack) {
        return true;
    } else {
        return false;
    }
}
static bool middleUser(User *currentUser)
{
    if(currentUser != userFront && currentUser != userBack) {
        return true;
    } else {
        return false;
    }
}
static bool lastUser(User *currentUser)
{
    if(currentUser != userFront && currentUser == userBack) {
        return true;
    } else {
        return false;
    }
}

void cleanUp(User *deleteUser)
{
    int cancelResult = pthread_cancel(deleteUser->thread);
    if (cancelResult != 0) {
        perror("Failed to cancel thread!\n");
    }
    if (pthread_join(deleteUser->thread, NULL) != 0) {
        perror("Failed to join threads!\n");
    }

    close(deleteUser->sock);    //Clean up the socket
    free(deleteUser);           //Free memory
}

void removeUser(User *currentUser)
{
    User *deleteUser = currentUser;

    if(onlyUser(currentUser)) {
        userFront = NULL;
        userBack = NULL;
    } else if(firstUser(currentUser)) {
        userFront = currentUser->next;
        currentUser->next->prev = NULL;
    } else if(middleUser(currentUser)) {
        currentUser->next->prev = currentUser->prev;
        currentUser->prev->next = currentUser->next;
    } else if(lastUser(currentUser)) {
        currentUser->prev->next = NULL;
        userBack = currentUser->prev;
    }
    unlockUser();
    cleanUp(deleteUser);
    
}

int initMutex() {
    int statusCode;
    statusCode = pthread_mutex_init(&userLock, NULL);
    return statusCode;
}

void lockUser(){
    pthread_mutex_lock(&userLock);
}

void unlockUser(){
    pthread_mutex_unlock(&userLock);
}

User *getUserByName(const char *name)
{
    User *currentUser = userFront;
    lockUser();
    while(currentUser != NULL)
    {
        printf("Checking user %s\n", currentUser->name);
        if(strcmp(currentUser->name, name) == 0)
        {
            unlockUser();
            printf("User %s found!\n", currentUser->name);
            return currentUser;
        }
        currentUser = currentUser->next;
    }
    unlockUser();
    printf("User not found!\n");
    return NULL;
}