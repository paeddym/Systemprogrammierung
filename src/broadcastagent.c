#include <pthread.h>
#include <mqueue.h>
#include "broadcastagent.h"
#include <util.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <semaphore.h>
#include <errno.h>
#include <stdio.h>


static mqd_t messageQueue;
static pthread_t threadId;
struct mq_attr attribute;
static unsigned int priority = 0;
static sem_t sem;
uint8_t chatStatus = running;

static char *messageQueueName = "/msq";

int receiveMessage(int fd, Message *buffer){
    int connectionStatus = networkReceive(fd, buffer);
    if (connectionStatus <= communicationError) {
        errorPrint("Error while receiving message %d ", connectionStatus);
    }
    return connectionStatus;
}

void broadcastMessage(Message *buffer, User *skipThisUser){
	iterateOverSockets(networkSend, skipThisUser, buffer);
}

void sendMessage(User *myUser, Message *buffer){
	networkSend(myUser->sock, buffer);
}

static void *broadcastAgent(void *arg)
{
	Message serverToClient;
	while (1) {
		if (getChatStatus() == running) {
			struct mq_attr queueAttr;
			int status = mq_getattr(messageQueue, &queueAttr);
			if (status == -1) {
				perror("mq_getattr error");
				break;
			}
			if (queueAttr.mq_curmsgs > 0) {
				printf("Message in queue\n");
				ssize_t bytesRead = mq_receive(messageQueue, ((char *)&serverToClient), attribute.mq_msgsize, &priority);
				if (bytesRead == -1) {
					perror("mq_receive error");
					break;		
				}
				broadcastMessage(&serverToClient, NULL);
			}
		}
	}
	return arg;
}

int broadcastAgentInit(void)
{
	if(sem_init(&sem, pshared, running) != 0) {
        errorPrint("Failed to create semaphore!");
        return -1;
    }
	attribute.mq_maxmsg = 10;
	attribute.mq_msgsize = messageMax;
	int flags = O_CREAT | O_RDWR;
	int mode = 0666;
	messageQueue = mq_open(messageQueueName, flags, mode, &attribute);
    if (messageQueue == (mqd_t)-1) {
        errorPrint("mq_open error");
        return -1;
    }

	int thread_result = pthread_create(&threadId, NULL, broadcastAgent, NULL);
	if (thread_result != 0) {
		errorPrint("Failed to create thread!\n");
		return -1;
	}
	return 0;
}

void broadcastAgentCleanup(void){
	pthread_cancel(threadId);
	mq_close(messageQueue);
	mq_unlink(messageQueueName);
	sem_destroy(&sem);
}


void sendToMessageQueue(Message *buffer, User *user) {
    Message msg = initMessage(serverToClientType);

    struct timespec ts;
    ts.tv_sec = 0;
    ts.tv_nsec = 0U;

	mq_timedsend(messageQueue, (char *)buffer, attribute.mq_msgsize, priority, &ts);
	if (errno == ETIMEDOUT) {
		msg.body.serverToClient.timestamp = (uint64_t)time(NULL);
		msg.body.serverToClient.originalSender[0] = '\0';
		createMessage(&msg, "Chat is paused and the send queue is full!");
		sendMessage(user, &msg);
	}
}

void pauseChat(){
	sem_wait(&sem);
	chatStatus = paused;	
}

void resumeChat(){
	sem_post(&sem);
	chatStatus = running;
}

uint8_t getChatStatus() {
    return chatStatus;
}
