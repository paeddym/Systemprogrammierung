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
	if(buffer->header.type == loginRequestType){
		length = buffer->header.length - sizeof(buffer->body.loginRequest.magic) - sizeof(buffer->body.loginRequest.version);
		return length;
	}
	if(buffer->header.type == clientToServerType){
		length = buffer->header.length;
		return length;
	} else {
		return error;
	}
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
	Message userRemoved = initMessage(userRemovedType);
	userRemoved.body.removedUser.code = code;
	createMessage(&userRemoved, user->name);
	printf("%s has left the chat\n", user->name);
	removeUser(user);
}

int handleLogin(User *self, char *name, Message *loginRequest){
	Message loginResponse = initMessage(loginResponseType);
	const char serverName[nameMax] = "Server\0";
	int code = checkLoginRequest(name, loginRequest->body.loginRequest.version);
	loginResponse.body.loginResponse.code = code;
	createMessage(&loginResponse, serverName);
	sendMessage(self, &loginResponse);
	return code;
}

void *clientthread(void *arg){
	User *self = (User *)arg;
	debugPrint("Started client thread");

	//Receiving Login Request
	Message loginRequest;
	int code = receiveMessage(self->socket, &loginRequest);
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
		unlockUser();
		cleanUp(self);
		return NULL;
	}

	addUser(self);
	strcpy(self->name, name);
	printf("Added user %s\n", name);


	//Send new user data to all users
	Message userData = initMessage(userAddedType);
	createMessage(&userData, self->name);
	broadcastMessage(&userData, NULL);
	printf("%s has joined the chat\n", self->name);

	
	//Send data of registered users to new user
	User *currentUser = getFirstUser();
	while(currentUser->next != NULL){
		if(currentUser != self){
			userData = initMessage(userAddedType);
			createMessage(&userData, currentUser->name);
			userData.body.addedUser.timestamp = 0;
			sendMessage(self, &userData);
		}
		currentUser = currentUser->next;
	}

	unlockUser();

	//Enter chat loop
	Message clientToServer, serverToClient;
	uint8_t userRemovedCode;
	char buffer[textMax];

	while(1){
		serverToClient = initMessage(serverToClientType);
		memset(buffer, 0, sizeof(buffer));
		memset(clientToServer.body.clientToServer.text, 0, sizeof(clientToServer.body.clientToServer.text));
		memset(serverToClient.body.serverToClient.text, 0, sizeof(serverToClient.body.serverToClient.text));

		int code = receiveMessage(self->socket, &clientToServer);
		if(code == clientClosedConnectionError){
			printf("Connection closed by client\n");
			userRemovedCode = closedByClient;
			break;
		} else if(code == error){
			printf("Error receiving message\n");
			userRemovedCode = connectionError;
			break;
		}

		int textLength = getHeaderLength(&clientToServer);
		memcpy(buffer, clientToServer.body.clientToServer.text, textLength);
		buffer[textLength] = '\0';

		if(buffer[0] != '/'){
			strcpy(serverToClient.body.serverToClient.sender, self->name);
			createMessage(&serverToClient, buffer);
			sendToMessageQueue(&serverToClient, self);
			continue;
		}

		//Commands
	}
	
	printf("%s has left the chat\n", self->name);
	handleUserRemoved(self, userRemovedCode);

	return NULL;
}
