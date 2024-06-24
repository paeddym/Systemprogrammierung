#include "clientthread.h"
#include "user.h"
#include "util.h"
#include "network.h"
#include <string.h> 
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h> 
#include "broadcastagent.h"

//Functions

int getHeaderLength(Message *buffer){
	int length;
	if(buffer->header.type == MSG_LOGIN_REQUEST){
		length = buffer->header.length - sizeof(buffer->body.loginRequest.magic) - sizeof(buffer->body.loginRequest.version);
		return length;
	}
	if(buffer->header.type == MSG_CLIENT_TO_SERVER){
		length = buffer->header.length;
		return length;
	}
	return error;
}

int checkLoginRequest(const char *clientName, uint8_t version){
	if(version != 0){
		return loginProtocolMismatch;
	}

	for(int i = 0; clientName[i]; i++) {
		if(clientName[i] < 33 || clientName[i] >= 126 || clientName[i] == 34 || clientName[i] == 37 || clientName[i] == 96){
			return loginInvalidName;
		}
	}
	return (getUserByName(clientName) == NULL) ? loginSuccess : loginNameTaken;
}

void handleUserRemoved(User *user, int code){
	lockUser();
	Message userRemoved = initMessage(MSG_REMOVED_USER);
	userRemoved.body.removedUser.code = code;
	createMessage(&userRemoved, user->name);
	printf("%s has left the chat\n", user->name);
	removeUser(user);
}

int handleLogin(User *self, char *name, Message *loginRequest){
	Message loginResponse = initMessage(MSG_LOGIN_RESPONSE);
	const char serverName[nameMax] = "Server\0";
	int code = checkLoginRequest(name, loginRequest->body.loginRequest.version);
	loginResponse.body.loginResponse.code = code;
	createMessage(&loginResponse, serverName);
	send(self, &loginResponse);
	return code;
}

//Client Thread

void *clientthread(void *arg){
	User *self = (User *)arg;
	debugPrint("Started client thread");

	//Receiving Login Request
	Message loginRequest;
	int code = receive(self->sock, &loginRequest);
	if(code != 0){
		errorPrint("Login Error");
		unlockUser();
		cleanUp(self);
		return NULL;
	}
	printf("Receiving Login Request\n");

	char name[nameMax];
	int nameLength = getHeaderLength(&loginRequest);
	memcpy(name, loginRequest.body.loginRequest.name, nameLength);
	name[nameLength] = '\0';
	printf("Name: %s\n", name);


	//Send Login Response
	code = handleLogin(self, name, &loginRequest);
	if(code != loginSuccess){
		errorPrint("Login Error");
		unlockUser();
		cleanUp(self);
		return NULL;
	}

	addUser(self);
	strcpy(self->name, name);
	printf("Added user %s\n", name);

	//TODO: Send added user message to all users

	return NULL;
}
