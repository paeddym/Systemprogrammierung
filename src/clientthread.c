#include <string.h> 
#include <unistd.h> 
#include "clientthread.h"
#include "user.h"
#include "util.h"
#include "network.h"
#include "broadcastagent.h"
#include <pthread.h>
#include <string.h>
#include <stdio.h>


int getHeaderLength(Message *buffer){
	int length;
	if(buffer->header.type == loginRequestType){
		length = buffer->header.length - sizeof(buffer->body.loginRequest.magic) - sizeof(buffer->body.loginRequest.version);
		return length;
	}
	if(buffer->header.type == clientToServerType){
		length = buffer->header.length;
		return length;
	}
	else{
		return error;
	}
}

int checkLoginRequest(const char *clientName, uint8_t clientVersion) {
    if (clientVersion != 0) {
        return protocolVersionMismatch;
    }
    for (int i = 0; clientName[i]; i++) {
        if (clientName[i] < 33 || clientName[i] >= 126 || clientName[i] == 34 || clientName[i] == 37 || clientName[i] == 96) {
            return nameInvalid;
        }
    }
    return (getUserByName(clientName) == NULL) ? success : nameAlreadyTaken;
}

void extractUserName(const char *text, char *userName) {
    const char *start = text + 6;
    const char *end = strchr(start, ' ');
	if (end == NULL){
        end = strchr(start, '\0');
    }
    strncpy(userName, start, end - start);
    userName[end - start] = '\0';
}

void handleUserRemoval(User *userToRemove, int urmCode){
	lockUser();
	Message urm = initMessage(userRemovedType);
	urm.body.userRemoved.code = urmCode;
	createMessage(&urm, userToRemove->name);
	broadcastMessage(&urm, userToRemove);
	printf("User %s has left the chat.\n", userToRemove->name);
	removeUser(userToRemove);
}

int handleLogin(User *self, char *clientName, Message *lrq) {
	Message lre = initMessage(loginResponseType); 
	const char serverName[nameMax] = "chatbot-server\0";
	int clientValidation = checkLoginRequest(clientName, lrq->body.loginRequest.version);
	lre.body.loginResponse.code = clientValidation;
	createMessage(&lre, serverName);
	sendMessage(self, &lre);
	return clientValidation;
}

void *clientthread(void *arg)
{
	User *self = (User *)arg;
	debugPrint("Client thread started.");


//LoginRequest
	Message lrq;
	int statusCode = receiveMessage(self->sock, &lrq); 
	if (statusCode <= communicationError) {
        errorPrint("Login Request Error");
		unlockUser();
		userCleanUp(self);
		return NULL;
    }
	printf("Login Request received.\n");

	char clientName[nameMax];
	int lengthOfClientName = getHeaderLength(&lrq);
	memcpy(clientName, lrq.body.loginRequest.name, lengthOfClientName); 
	clientName[lengthOfClientName] = '\0';
	printf("Clientname: %s\n", clientName);

	
//LoginResponse
	int clientValidation = handleLogin(self, clientName, &lrq);

	printf("clientValidation %d\n", clientValidation);
	if(clientValidation != success){
		unlockUser();
		userCleanUp(self);
		return NULL;
	}

	addUser(self);

	strcpy(self->name , clientName);
	printf("self->name %s\n", self->name);


//Send new UserData to other users
	Message uad = initMessage(userAddedType);
	createMessage(&uad, self->name);
	broadcastMessage(&uad, NULL);
	printf("User %s has joined the chat.\n", self->name);


//Send new User the Data of the other Users
	User *loggedInUser = getFirstUser();
	while(loggedInUser->next != NULL){
		if(loggedInUser != self){
			uad = initMessage(userAddedType);
			createMessage(&uad, loggedInUser->name);
			uad.body.userAdded.timestamp = 0;
			sendMessage(self, &uad);
		}
		loggedInUser = loggedInUser->next;
	}
	unlockUser();


//Mailserver loop
	Message clientToServer, serverToClient;
	uint8_t urmCode;
	char textBuffer[textMax];

	while(1){
		serverToClient = initMessage(serverToClientType);

		memset(textBuffer, 0, sizeof(textBuffer));
		memset(clientToServer.body.clientToServer.text, 0, sizeof(clientToServer.body.clientToServer.text));
		memset(serverToClient.body.serverToClient.text, 0, sizeof(serverToClient.body.serverToClient.text));
		
		int statusCode = receiveMessage(self->sock, &clientToServer);
		if(statusCode == clientClosedConnectionError){
			printf("Client closed connection in mail loop%d ", statusCode);
			urmCode	= connectionClosedByClient;
			break;
		}
		if(statusCode == error){
			printf("Communication error in mail loop%d ", statusCode);
			urmCode = communicationError;
			break;
		}

		int lengthOfText = getHeaderLength(&clientToServer);
		memcpy(textBuffer, clientToServer.body.clientToServer.text, lengthOfText);
		textBuffer[lengthOfText] = '\0';

		//Message
		if(textBuffer[0] != '/'){
			strcpy(serverToClient.body.serverToClient.originalSender, self->name);
			createMessage(&serverToClient, textBuffer);
			sendToMessageQueue(&serverToClient, self);
			continue;
		}
		//Admin Commands
		serverToClient.body.serverToClient.originalSender[0] = '\0';

		int commandCode = invalidCommandCode;
		if (strncmp(textBuffer, "/kick ", 6) == 0 && lengthOfText > 6) {
			commandCode = kickClientCommandCode;
		} else if (strncmp(textBuffer, "/pause", 6) == 0) {
			commandCode =  pauseChatCommandCode;
		} else if (strncmp(textBuffer, "/resume", 7) == 0) {
			commandCode =  resumeChatCommandCode;
		}
		
		if(commandCode == invalidCommandCode){
			createMessage(&serverToClient, "Invalid command!");
			sendMessage(self, &serverToClient);
			continue;
		}

		if (strcmp(self->name, "Admin") != 0) {
			if (commandCode == kickClientCommandCode) {
				strcpy(textBuffer, "You must be administrator to use the /kick Command!");
			} else if (commandCode == pauseChatCommandCode) {
				strcpy(textBuffer, "You must be administrator to use the /pause Command!");
			} else if (commandCode == resumeChatCommandCode) {
				strcpy(textBuffer, "You must be administrator to use the /resume Command!");
			}
			createMessage(&serverToClient, textBuffer);
			sendMessage(self, &serverToClient);
			continue;
		}

		if(commandCode == kickClientCommandCode) {
			printf("kickClientCommandCode\n");
			char userNameToKick[nameMax];
			extractUserName(clientToServer.body.clientToServer.text, userNameToKick);
			if(strcmp(userNameToKick, "Admin") == 0){
				createMessage(&serverToClient, "You can't kick yourself!");
				sendMessage(self, &serverToClient);
				continue;
			}
			User *userToKick = getUserByName(userNameToKick);
			if(userToKick == NULL){
				printf("userToKick == NULL");
				createMessage(&serverToClient, "User not found!");
				sendMessage(self, &serverToClient);
				continue;
			}
			printf("User %s has been kicked by %s.\n", userToKick->name, self->name);
			handleUserRemoval(userToKick, kickedFromTheServer);
			continue;
		}
		else if(commandCode == pauseChatCommandCode) {
			if(getChatStatus() == paused){
				createMessage(&serverToClient, "Chat is already paused!");
				sendMessage(self, &serverToClient);
				continue;
			}
			pauseChat();
			createMessage(&serverToClient, "The chat has been paused");
			broadcastMessage(&serverToClient, NULL);
			continue;
		}
		else if(commandCode == resumeChatCommandCode) {
			if(getChatStatus() != paused){
				createMessage(&serverToClient, "Chat is not paused!");
				sendMessage(self, &serverToClient);
				continue;
			}
			resumeChat();
			createMessage(&serverToClient, "The chat has been resumed");
			broadcastMessage(&serverToClient, NULL);
			continue;						
		}
	}
	printf("User %s has left the chat.\n", self->name);
	handleUserRemoval(self, urmCode);

	debugPrint("Client thread stopping.");

	return NULL;
}