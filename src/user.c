#include <pthread.h>
#include <errno.h>
#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h> 
#include <string.h> 
#include "user.h"
#include "clientthread.h"
#include "network.h"

static pthread_mutex_t userLock = PTHREAD_MUTEX_INITIALIZER;
static User *userFront = NULL;
static User *userBack = NULL;

void lockUser(){
    pthread_mutex_lock(&userLock);
}

void unLockUser(){
    pthread_mutex_unlock(&userLock);
}

int initMutex() {
    int statusCode;
    statusCode = pthread_mutex_init(&userLock, NULL);
    return statusCode;
}

User *allocateSpaceForUser()
{
    User *newUser = (User *)malloc(sizeof(User));
    if(newUser == NULL)
    {
        errno = ENOMEM;
        perror("Memory allocation fail");
        return NULL;
        //todo errorhandling
    }
    return newUser;
}

User *addUser(User *newUser)
{
    if(userFront == NULL){
        newUser->prev = NULL;
        newUser->next = NULL;
        userFront = newUser;
        userBack = newUser;
    }
    else{
        userBack->next = newUser;
        newUser->prev = userBack;
        newUser->next = NULL;
        userBack = newUser;
    }
    return newUser;
}

User *getUserTroughName(const char *name)
{
    User *currentUser = userFront;
    while(currentUser != NULL)
    {
        printf("Checking user: %s\n", currentUser->name);
        if(strcmp(name, currentUser->name) == 0)
        {
            printf("User found: %s\n", currentUser->name);
            return currentUser;
        }
        currentUser = currentUser->next;
    }
    printf("User not found\n"); 
    return NULL;
}

void iterateOverSockets(int (*func)(int, const Message *), User *self, void *buffer) {
    for (User *currentUser = userFront; currentUser != NULL; currentUser = currentUser->next) {
        if (currentUser != self) {
            printf("send to: %s", currentUser->name);
            func(currentUser->sock, buffer);
        }
    }
}

static bool onlyUser(User *currentUser)
{
    if(currentUser == userFront && currentUser == userBack){
        return true;
    }else{
        return false;
    }
}
static bool firstUser(User *currentUser)
{
    if(currentUser == userFront && currentUser != userBack){
        return true;
    }else{
        return false;
    }
}
static bool middleUser(User *currentUser)
{
    if(currentUser != userFront && currentUser != userBack){
        return true;
    }else{
        return false;
    }
}
static bool lastUser(User *currentUser)
{
    if(currentUser != userFront && currentUser == userBack){
        return true;
    }else{
        return false;
    }
}

void cleanUpOfUser(User *deleteUser)
{
    //Thread cleanup
    int cancelResult = pthread_cancel(deleteUser->thread);
    if (cancelResult != 0)
    {
        perror("Thread cancellation failed\n");
    }
    if (pthread_join(deleteUser->thread, NULL) != 0)
    {
        perror("Thread join failed\n");
    }
    //Socket cleanup
    close(deleteUser->sock);
    //Memory cleanup
    free(deleteUser);
    
}

void removeUser(User *currentUser)
{
    User *deleteUser = currentUser;
    if(onlyUser(currentUser)) 
    {
        userFront = NULL;
        userBack = NULL;
    }else if(firstUser(currentUser)){
        userFront = currentUser->next;
        currentUser->next->prev = NULL;
    }else if(middleUser(currentUser)){
        currentUser->next->prev = currentUser->prev;
        currentUser->prev->next = currentUser->next;
    }else if(lastUser(currentUser)){
        currentUser->prev->next = NULL;
        userBack = currentUser->prev;
    }
    unLockUser();
    cleanUpOfUser(deleteUser);
    
}
User *getFirstUser(){
    return userFront;
}
