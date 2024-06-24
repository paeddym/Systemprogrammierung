#include <pthread.h>
#include <mqueue.h>
#include <stdio.h>
#include <errno.h>
#include <errno.h>
#include "broadcastagent.h"
#include "util.h"
#include "network.h"
#include "user.h"

static mqd_t messageQueue;
static pthread_t threadId;
struct mq_attr attr;
static unsigned int priority = 0;

static char *messageQueueName = "/msq";

static void *broadcastAgent(void *arg)
{
	//TODO: Implement thread function for the broadcast agent here!
	return arg;
}

int broadcastAgentInit(void)
{

	//TODO: create message queue

	
	//TODO: start thread

	return -1;
}



void broadcastAgentCleanup(void)
{
	//TODO: Cleanup
}

int receive(int fd, Message *buffer){
    int connectionStatus = networkReceive(fd, buffer);
    if (connectionStatus <= connectionError) {
        errorPrint("Failed to receive message! %d ", connectionStatus);
    }
    return connectionStatus;
}



void send(User *myUser, Message *buffer){
	networkSend(myUser->sock, buffer);
}
