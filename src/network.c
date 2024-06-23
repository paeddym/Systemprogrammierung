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

	buffer->header.length = ntohs(buffer->header.length);	// Convert header to host byte order
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

	if(buffer->header.type == MSG_LOGIN_REQUEST){	// Convert magic number to host byte order
		buffer->body.loginRequest.magic = ntohl(buffer->body.loginRequest.magic);
	}

	if(isBodyValid(buffer) == false){
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
	if(type == MSG_LOGIN_REQUEST && (length <= 5 || length >= 37)){
		perror("Invalid login request header length!");
		return false;
	} else if(type == MSG_CLIENT_TO_SERVER && length >= 512) {
		perror("Invalid client to server header length!");
		return false;
	}
	return true;
}

bool isBodyValid(Message *buffer){
	if (buffer->header.type == MSG_LOGIN_REQUEST){
		if (buffer->body.loginRequest.magic != 0x0badf00d){
			perror("Invalid magic number in login request!");
			return false;
		}
	}
	return true;
}

Message setMessageType(uint8_t type){
	Message message;
	switch(type){
		case MSG_LOGIN_RESPONSE:
			message.header.type = MSG_LOGIN_RESPONSE;
			message.body.loginResponse.magic = 0x0c001c001;
			break;
		case MSG_SERVER_TO_CLIENT:
			message.header.type = MSG_SERVER_TO_CLIENT;
			break;
		case MSG_ADDED_USER:
			message.header.type = MSG_ADDED_USER;
			break;
		case MSG_REMOVED_USER:
			message.header.type = MSG_REMOVED_USER;
			break;
	}
	return message;

}

void setLength(Message *messageBuffer, int contentLength){
	uint16_t length = 0;
	switch (messageBuffer->header.type){
		case MSG_LOGIN_RESPONSE:
			length = sizeof(messageBuffer->body.loginResponse.magic) + sizeof(messageBuffer->body.loginResponse.code) + contentLength;
			break;
		case MSG_SERVER_TO_CLIENT:
			length = sizeof(messageBuffer->body.serverToClient.timestamp) + sizeof(messageBuffer->body.serverToClient.sender) + contentLength;
			break;
		case MSG_ADDED_USER:
			length = sizeof(messageBuffer->body.addedUser.timestamp) + contentLength;
			break;
		case MSG_REMOVED_USER:
			length = sizeof(messageBuffer->body.removedUser.timestamp) + sizeof(messageBuffer->body.removedUser.code) + contentLength;
			break;
	}
	messageBuffer->header.length = length;
}

void convertMessageToNetworkOrder(Message *messageBuffer){
	messageBuffer->header.length = htons(messageBuffer->header.length);
	
	switch(messageBuffer->header.type){
		case MSG_LOGIN_RESPONSE:
			messageBuffer->body.loginResponse.magic = htonl(messageBuffer->body.loginResponse.magic);
			break;
		case MSG_SERVER_TO_CLIENT:
			messageBuffer->body.serverToClient.timestamp = hton64u(messageBuffer->body.serverToClient.timestamp);
			break;
		case MSG_ADDED_USER:
			messageBuffer->body.addedUser.timestamp = hton64u(messageBuffer->body.addedUser.timestamp);
			break;
		case MSG_REMOVED_USER:
			messageBuffer->body.removedUser.timestamp = hton64u(messageBuffer->body.removedUser.timestamp);
			break;
	}
}

void createMessage(Message *messageBuffer, const char *textBuffer){
	int textLength = strlen(textBuffer);
	setLength(messageBuffer, textLength);
	uint64_t timestamp = time(NULL);

	switch(messageBuffer->header.type){
		case MSG_LOGIN_RESPONSE:
			memcpy(messageBuffer->body.loginResponse.name, textBuffer, textLength);
			convertMessageToNetworkOrder(messageBuffer);
			break;
		case MSG_SERVER_TO_CLIENT:
			messageBuffer->body.serverToClient.timestamp = timestamp;
			memcpy(messageBuffer->body.serverToClient.text, textBuffer, textLength);
			convertMessageToNetworkOrder(messageBuffer);
			break;
		case MSG_ADDED_USER:
			messageBuffer->body.addedUser.timestamp = timestamp;
			
			break;
		case MSG_REMOVED_USER:
			messageBuffer->body.removedUser.timestamp = timestamp;
			memcpy(messageBuffer->body.removedUser.name, textBuffer, textLength);
			convertMessageToNetworkOrder(messageBuffer);
			break;
	}
}
