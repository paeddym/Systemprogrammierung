#include <pthread.h>
#include "user.h"
#include "clientthread.h"
#include <errno.h>
#include <stdbool.h>

static pthread_mutex_t userLock = PTHREAD_MUTEX_INITIALIZER;
static User *userFront = NULL;
static User *userBack = NULL;

//TODO: Implement the functions declared in user.h

User *allocateSpace()
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

void removeUser(User *currentUser)
{
    User *deleteUser = currentUser;
    //Is onlyUser necessary? Because before free front and back need to be set to NULL
    if(onlyUser(currentUser)) 
    {
        userFront = NULL;
        userBack = NULL;
    }
    else if(firstUser(currentUser))
    {
        userFront = currentUser->next;
        currentUser->next->prev = NULL;
    }
    else if(middleUser(currentUser))
    {
        currentUser->next->prev = currentUser->prev;
        currentUser->prev->next = currentUser->next;
    }
    else if(lastUser(currentUser))
    {
        currentUser->prev->next = NULL;
        userBack = currentUser->prev;
    }
    unLockUser();
    cleanUpOfUser(deleteUser);
    
}

static bool onlyUser(User *currentUser)
{
    if(currentUser == userFront && currentUser == userBack)
    {
        return true;
    }
    else
    {
        return false;
    }
}
static bool firstUser(User *currentUser)
{
    if(currentUser == userFront && currentUser != userBack)
    {
        return true;
    }
    else
    {
        return false;
    }
}
static bool middleUser(User *currentUser)
{
    if(currentUser != userFront && currentUser != userBack)
    {
        return true;
    }
    else
    {
        return false;
    }
}
static bool lastUser(User *currentUser)
{
    if(currentUser != userFront && currentUser == userBack)
    {
        return true;
    }
    else
    {
        return false;
    }
}

void cleanUp(User *deleteUser)
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

int initMutex() {
    int statusCode;
    statusCode = pthread_mutex_init(&userLock, NULL);
    return statusCode;
}

void lockUser(){
    pthread_mutex_lock(&userLock);
}

void unLockUser(){
    pthread_mutex_unlock(&userLock);
}