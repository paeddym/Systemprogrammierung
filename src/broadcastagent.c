#include <pthread.h>
#include <mqueue.h>
#include <stdio.h>
#include <errno.h>
#include <errno.h>
#include "broadcastagent.h"
#include "util.h"
#include "network.h"
#include "user.h"
#include <semaphore.h>

static mqd_t messageQueue;
static pthread_t threadId;
struct mq_attr attr;
static unsigned int priority = 0;
static sem_t semaphore;

static char *messageQueueName = "/msq";

void broadcastMessage(Message *buffer, User *skipUser){
	iterateList(networkSend, skipUser, buffer);
}

static void *broadcastAgent(void *arg)
{
	//TODO: Implement thread function for the broadcast agent here!
	Message serverToClient;
	while (1) {
			struct mq_attr queueAttr;
			int status = mq_getattr(messageQueue, &queueAttr);
			if (status == -1) {
				perror("mq_getattr");
				break;
			}
			if (queueAttr.mq_curmsgs > 0) {
				printf("Message in queue\n");
				ssize_t bytesRead = mq_receive(messageQueue, ((char *)&serverToClient), attr.mq_msgsize, &priority);

				if (bytesRead == -1) {
					perror("mq_receive");
					break;		
				}

				broadcastMessage(&serverToClient, NULL);
			} 
	}
	return arg;

}

int broadcastAgentInit(void)
{
	if(sem_init(&semaphore, pshared, running) != 0){
		errorPrint("Failed to initialize semaphore!\n");
		return -1;
	}
	
	attr.mq_maxmsg = 10;
	attr.mq_msgsize = messageMax;
	int flags = O_CREAT | O_RDWR;
	int mode = 0666;
	messageQueue = mq_open(messageQueueName, flags, mode, &attr);
    if (messageQueue == (mqd_t)-1) {
        errorPrint("mq_open");
        return -1;
    }
	
	//TODO: start thread
	int thread_result = pthread_create(&threadId, NULL, broadcastAgent, NULL);
	if (thread_result != 0) {
		errorPrint("Thread creation failed\n");
		return -1;
	}
	return 0;
}

void sendMessage(User *user, Message *buffer){
	networkSend(user->socket, buffer);
}

void sendToMessageQueue(Message *buffer, User *user) {
    Message message = initMessage(serverToClientType);

    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 0U;

	mq_timedsend(messageQueue, (char *)buffer, attr.mq_msgsize, priority, &ts);
	if (errno == ETIMEDOUT) {
		message.body.serverToClient.timestamp = (uint64_t)time(NULL);
		message.body.serverToClient.sender[0] = '\0';
		createMessage(&message, "Chat is paused and the send queue is full!");
		sendMessage(user, &message);
	}
}

void broadcastAgentCleanup(void)
{
	pthread_cancel(threadId);
	mq_close(messageQueue);	
	mq_unlink(messageQueueName);
	sem_destroy(&semaphore);
}

int receiveMessage(int fd, Message *buffer){
    int connectionStatus = networkReceive(fd, buffer);
    if (connectionStatus != 0) {
        errorPrint("Failed to receive message! %d ", connectionStatus);
    }
    return connectionStatus;
}