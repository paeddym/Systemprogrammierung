#include <errno.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <stdio.h>
#include <time.h>
#include "network.h"
#include "connectionhandler.h"
#include "util.h"
#include <string.h>

static int checkHeader(uint8_t type, uint16_t length);
static int checkBody(Message *buffer);


int networkReceive(int fd, Message *buffer)
{
	int errorCode = noError;

	ssize_t bytesReceived = recv(fd, &buffer->header, sizeof(buffer->header), MSG_WAITALL);
	
	printf("Receiving message...\n");
	printf("Type: %d\n", buffer->header.type);
	printf("Length: %d\n", buffer->header.length);

	if(bytesReceived == 0){
		perror("client closed connection");
		return clientClosedConnectionError;
	}
	else if(bytesReceived == -1){
		perror("Failed to receive!");
		return error;
	}

	buffer->header.length = ntohs(buffer->header.length);

	if(checkHeader(buffer->header.type, buffer->header.length) != noError){
		perror("Invalid length or type");
		return error;
	}

	bytesReceived = recv(fd, &buffer->body, buffer->header.length, MSG_WAITALL);
	printf("Bytes received: %ld\n", bytesReceived);
	if(bytesReceived == 0){
		perror("Client closed connection");
		return clientClosedConnectionError;
	}
	else if(bytesReceived == -1){
		perror("Failed to receive!");
		return error;
	}

	printf("Text: %s\n", buffer->body.clientToServer.text);

	if(buffer->header.type == loginRequestType){
		buffer->body.loginRequest.magic = ntohl(buffer->body.loginRequest.magic);
	}
	
	if(checkBody(buffer) != noError){
		errorCode = error;
	}
	return errorCode;
}

int networkSend(int fd, const Message *buffer)
{
	int errorCode = noError;

	printf("Sending message...\n");
	printf("Type: %d\n", buffer->header.type);
	printf("Length: %d\n", buffer->header.length);
	printf("text: %s\n", buffer->body.serverToClient.text);

	ssize_t bytesSent = send(fd, &buffer->header, sizeof(buffer->header), 0);

	if(bytesSent == -1){
		perror("Failed to send header!");
		errorCode = error;
	}

	bytesSent = send(fd, &buffer->body, ntohs(buffer->header.length), 0);
	if(bytesSent == -1){
		perror("Failed to send body!");
		errorCode = error;
	}
	return errorCode;
}

static int checkHeader(uint8_t type, uint16_t length){
	if(type == loginRequestType){
		if(length <= 5 || length >= 37){
			perror("Invalid loginRequest length!");
			return error;
		}
		return noError;
	}
	else if(type == clientToServerType){
		if(length > 512){
			perror("Invalid message length!");
			return error;
		}
		return noError;
	}
	return error;
}

static int checkBody(Message *buffer){
	if(buffer->header.type == loginRequestType){
		if(buffer->body.loginRequest.magic != 0x0badf00d){
			perror("Invalid magic number!");
			return error;
		}
	}
	return noError;
}

void setMessageLength(Message *buffer, int stringLength) {
    uint16_t len = 0;
    switch (buffer->header.type) {
        case loginResponseType:
            len = sizeof(buffer->body.loginResponse.magic) + sizeof(buffer->body.loginResponse.code) + stringLength;
            break;
        case serverToClientType:
            len = sizeof(buffer->body.serverToClient.timestamp) + sizeof(buffer->body.serverToClient.originalSender) + stringLength;
            break;
        case userAddedType:
            len = sizeof(buffer->body.userAdded.timestamp) + stringLength;
            break;
        case userRemovedType:
            len = sizeof(buffer->body.userRemoved.timestamp) + sizeof(buffer->body.userRemoved.code) + stringLength;
            break;
    }
    buffer->header.length = len;
}


Message initMessage(uint8_t type) {
    Message newMessage;
	
    switch (type) {
        case loginResponseType:
            newMessage.header.type = loginResponseType;
            newMessage.body.loginResponse.magic = 0xc001c001;
            break;
        case serverToClientType:
            newMessage.header.type = serverToClientType;
            break;
        case userAddedType:
            newMessage.header.type = userAddedType;
            break;
        case userRemovedType:
            newMessage.header.type = userRemovedType;
            break;
    }
    return newMessage;
}

void createMessage(Message *messageBuffer, const char *textBuffer){
	
	int lengthOfText = strlen(textBuffer);
	setMessageLength(messageBuffer, lengthOfText);
	uint64_t currentTime = time(NULL);
	switch (messageBuffer->header.type) {
		case loginResponseType:
			memcpy(messageBuffer->body.loginResponse.sName, textBuffer, lengthOfText);
			prepareMessage(messageBuffer);
			break;
		case serverToClientType:
			messageBuffer->body.serverToClient.timestamp = currentTime;
			memcpy(messageBuffer->body.serverToClient.text, textBuffer, lengthOfText);
			prepareMessage(messageBuffer);
			break;
		case userAddedType:
			messageBuffer->body.userAdded.timestamp = currentTime;
			memcpy(messageBuffer->body.userAdded.name, textBuffer, lengthOfText);
			prepareMessage(messageBuffer);
			break;
		case userRemovedType:
			messageBuffer->body.userRemoved.timestamp = currentTime;
			memcpy(messageBuffer->body.userRemoved.name, textBuffer, lengthOfText);
			prepareMessage(messageBuffer);
			break;
	}
}

void prepareMessage(Message *buffer) {

    buffer->header.length = htons(buffer->header.length);
    switch (buffer->header.type) {
        case loginResponseType:
            buffer->body.loginResponse.magic = htonl(buffer->body.loginResponse.magic);
            break;
        case serverToClientType:
            buffer->body.serverToClient.timestamp = hton64u(buffer->body.serverToClient.timestamp);
            break;
        case userAddedType:
            buffer->body.userAdded.timestamp = hton64u(buffer->body.userAdded.timestamp);
            break;
        case userRemovedType:
            buffer->body.userRemoved.timestamp = hton64u(buffer->body.userRemoved.timestamp);
            break;
    }
}
