#include "clientthread.h"
#include "user.h"
#include "util.h"
#include <string.h> 


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
	memcpy(clientName, loginRequest.body.loginRequest.requestName, nameLength);
	lockUser();
	if(status != noError){
		close(self->sock);
		unlockUser();
		return NULL;
	}

	//Sending LoginResponse
	Message loginResponse;
	char *sName = "chatbot server";
	loginResponse = initMessage(loginResponseType);
	loginResponse.body.loginResponse.code = 0;
	loginResponse.header.length = sizeof(loginResponse.body.loginResponse.code) + sizeof(loginResponse.body.loginResponse.responseName) + sizeof(loginResponse.body.loginResponse.magic);
	memcpy(loginResponse.body.loginResponse.responseName, sName, strlen(sName));
	memcpy(self->name , clientName, strlen(clientName));//name gespeichert im user

	//user added

	while(1){
		//Receive messages
		Message clientToServer;
		int status = networkReceive(self->sock, &clientToServer);
		if(status != noError){
			//todo
		}

		char *command = extractCommand(clientToServer.body.clientToServer.text);

		Message serverToClient;
		serverToClient = initMessage(server2ClientType); //type and timestamp set
		strncpy(serverToClient.body.serverToClient.sender, '\0', sizeof(char));

		if(clientToServer.body.clientToServer.text[0] == '/'){//Befehl
			
			int admin = 1;
			if(admin == 1/*admin*/){
				if(command == 'kick'){
					//todo user kicken
					//original sender setzen?
				}
				else if(command == 'pause'){
					//serverToClient message
					strncpy(serverToClient.body.serverToClient.text, "the chat has been paused", sizeof(serverToClient.body.serverToClient.text));
				}
				else if(command == 'resume'){
					//serverToClient message
					strncpy(serverToClient.body.serverToClient.text, "the chat has been resumed", sizeof(serverToClient.body.serverToClient.text));
				}
			}
		}
		else if(clientToServer.body.clientToServer.text[0] != '/'){
			memcpy(serverToClient.body.serverToClient.sender, self->name, strlen(self->name));
			memcpy(serverToClient.body.serverToClient.text, clientToServer.body.clientToServer.text, strlen(clientToServer.body.clientToServer.text));
			iterateUsers(networkSend, self, &serverToClient);
		}

		serverToClient.header.length = sizeof(serverToClient.body.serverToClient.timestamp) + sizeof(serverToClient.body.serverToClient.sender) + sizeof(serverToClient.body.serverToClient.text);
	}

	if(clientName[0] == '\0'){
		close(self->sock);
		return NULL;
	}

	free(buffer);
	debugPrint("Client thread stopping.");
	return NULL;
}

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

const char* extractCommand(const char *text) {
    const char *start = strchr(text, '/');
    if (start != NULL) {
        start++; // Gehe zum Zeichen nach dem Slash
        const char *end = strchr(start, ' '); 
        if (end == NULL) {
            end = strchr(start, '\0'); // Wenn kein Leerzeichen gefunden wird, gehe bis zum String-Ende
        }
        static char command[20];
        strncpy(command, start, end - start);
        command[end - start] = '\0';
        return command;
    } else {
        //todo
    }
}

void sendMessageToClient(User *currentUser, char *buf) {
	int bytes_sent = send(currentUser->sock, buf, strlen(buf), 0);
	if (bytes_sent == -1) {
		perror("Failed to send message!");
	}
}
