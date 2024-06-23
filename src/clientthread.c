#include "clientthread.h"
#include "user.h"
#include "util.h"
#include "network.h"
#include <string.h> 
#include <stdlib.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h> 

int lengthOfName(Message *buffer){
	int length;
	if(buffer->header.type == loginRequestType){
		length = buffer->header.length - sizeof(buffer->body.loginRequest.magic) - sizeof(buffer->body.loginRequest.version);
		return length;
	}
	if(buffer->header.type == client2ServerType){
		length = buffer->header.length;
		return length;
	}
	return error;
}


void *clientthread(void *arg){
	User *self = (User *)arg;

	debugPrint("Started client thread");

	Message *buffer = calloc(1, sizeof(Message));
	if (buffer == NULL)
	{
    	perror("Memory allocation failed");
    	return NULL;
	}

	//Receive LoginRequest
	Message loginRequest;
	int status = networkReceive(self->sock, &loginRequest);
	int nameLength = lengthOfName(&loginRequest);
	char clientName[nameMax];
	memcpy(clientName, loginRequest.body.loginRequest.name, nameLength);
	lockUser();
	if(status != noError){
		close(self->sock);
		unlockUser();
		return NULL;
	}

	Message loginResponse;
	char *name = "Server";
	loginResponse = initMessage(loginResponseType);
	loginResponse.body.loginResponse.code = 0;
	loginResponse.header.length = sizeof(loginResponse.body.loginResponse.code) + sizeof(loginResponse.body.loginResponse.name) + sizeof(loginResponse.body.loginResponse.magic);
	memcpy(loginResponse.body.loginResponse.name, name, strlen(name));
	memcpy(self->name , clientName, strlen(clientName));//name gespeichert im user


	while(1){
		Message clientToServer;
		int status = networkReceive(self->sock, &clientToServer);
		if(status != noError){
			//TODO
		}

		Message serverToClient;
		serverToClient = initMessage(server2ClientType); //type and timestamp set
		strncpy(serverToClient.body.serverToClient.sender, '\0', sizeof(char));

		serverToClient.header.length = sizeof(serverToClient.body.serverToClient.timestamp) + sizeof(serverToClient.body.serverToClient.sender) + sizeof(serverToClient.body.serverToClient.text);

		if(clientToServer.body.clientToServer.text[0] != '/'){//Normale Nachricht

			//serverToClient message

			//Send them to all users, skip self
			
			//memset(serverToClient.body.serverToClient.sender, '\0', 32); // Mit Nullen initialisieren
			memcpy(serverToClient.body.serverToClient.sender, self->name, strlen(self->name));
			memcpy(serverToClient.body.serverToClient.text, clientToServer.body.clientToServer.text, strlen(clientToServer.body.clientToServer.text));
			iterateList(networkSend, self, &serverToClient);
		}
	}

	if(clientName[0] == '\0'){
		close(self->sock);
		return NULL;
	}

	free(buffer);
	debugPrint("Stopped client thread");
	return NULL;
}
