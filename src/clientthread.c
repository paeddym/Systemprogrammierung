#include "clientthread.h"
#include "user.h"
#include "util.h"
#include <string.h> 

#define MAX_BUFFER_SIZE  1024


void *clientthread(void *arg){
	User *self = (User *)arg;

	debugPrint("Started client thread");

	int bytes_received;
	char buf[MAX_BUFFER_SIZE];

	while(1)
	{
		bytes_received = recv(self->sock, buf, MAX_BUFFER_SIZE -1, 0);
		if (bytes_received == -1) {
    		perror("Failed to receive socket data!");
		} else if (bytes_received == 0) {
			removeUser(self);
			perror("Closed connection by peer");
		} else {
			buf[bytes_received] = '\0';
			iterateList(sendMessageToClient, self, buf);
		}			
	}
	debugPrint("Client thread stopping.");
	return NULL;
}

void sendMessageToClient(User *currentUser, char *buf) {
	int bytes_sent = send(currentUser->sock, buf, strlen(buf), 0);
	if (bytes_sent == -1) {
		perror("Failed to send message!");
	}
}
