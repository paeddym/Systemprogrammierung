#include <errno.h>
#include "network.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdbool.h>

int networkReceive(int fd, Message *buffer)
{
	ssize_t receivedBytes = recv(fd, &buffer->header, sizeof(buffer->header), MSG_WAITALL);

	printf("Receiving message with length %d and type %d\n", buffer->header.length, buffer->header.type);

	if (receivedBytes == -1) {
		perror("Failed to receive message!");
		return -1;
	} else if (receivedBytes == 0) {
		perror("Connection has been closed by the client!");
		return -1;
	}

	buffer->header.length = ntohs(buffer->header.length);	// Convert to host byte order
	if(isHeaderValid(buffer->header.type, buffer->header.length) == false){
		perror("Message with invalid header received!");
		return -1;
	}


	receivedBytes = recv(fd, &buffer->body, buffer->header.length, MSG_WAITALL);
	printf("Received %ld bytes\n", receivedBytes);
	if(receivedBytes == 0){
		perror("Connection has been closed by the client!");
		return -1;
	} else if(receivedBytes == -1){
		perror("Failed to receive message!");
		return -1;
	}
	printf("Message content: %s\n", buffer->body.clientToServer.text);

	if(isMessageValid(buffer) == false){
		perror("Message with invalid body received!");
		return -1;
	}

	return -1;
}

int networkSend(int fd, const Message *buffer)
{
	printf("Sending message:\nlength = %d\ntype = %d\ncontent = %s\n", buffer->header.length, buffer->header.type, buffer->body.serverToClient.text);

	ssize_t sentBytes = send(fd, &buffer->header, sizeof(buffer->header), 0);
	if (sentBytes == -1) {
		perror("Failed to send message header!");
		return -1;
	}

	sentBytes = send(fd, &buffer->body, buffer->header.length, 0);
	if (sentBytes == -1) {
		perror("Failed to send message body!");
		return -1;
	}

	return -1;
}

bool isHeaderValid(uint8_t type, uint16_t length){
	//Implement validation
	return false;
}

bool isMessageValid(Message *buffer){
	//Implement validation
	return false;
}
