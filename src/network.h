#ifndef CHAT_PROTOCOL_H
#define CHAT_PROTOCOL_H

#include <stdint.h>

enum { MSG_MAX = 1024 };

enum MESSAGE_LIMITS {
    nameMin = 1,
    nameMax = 31,
    textMax = 512,
	senderMax = 32,
	headerMax = 3,
    bodyMax = 552,
	messageMax = headerMax + bodyMax + 1
};

enum MESSAGE_TYPES {
    loginRequestType = 0,
    loginResponseType = 1,
    clientToServerType = 2,
    serverToClientType = 3,
    userAddedType = 4,
    userRemovedType = 5
};

enum ERROR_CODES {
    noError = 0,
    clientClosedConnectionError = 1,
    error = -1
};

enum USER_REMOVED_CODES {
    closedByClient = 0,
    kickedByAdmin = 1, 
    connectionError = 2
};

enum LOGIN_RESPONSE_CODES {
	loginSuccess = 0,
	loginNameTaken = 1,
	loginInvalidName = 2,
	loginProtocolMismatch = 3,
	loginError = 4
};

typedef struct __attribute__((packed))
{
	uint32_t magic;
	uint8_t version;
	char name[nameMax];
} LoginRequest;

typedef struct __attribute__((packed))
{
	uint32_t magic;
	uint8_t code;
	char name[nameMax];
} LoginResponse;

typedef struct __attribute__((packed))
{
	char text[textMax];
} ClientToServer;

typedef struct __attribute__((packed))
{
	uint64_t timestamp;
	char sender[senderMax];
	char text[textMax];
} ServerToClient;

typedef struct __attribute__((packed))
{
	uint64_t timestamp;
	char name[nameMax];
} AddedUser;

typedef struct __attribute__((packed))
{
	uint64_t timestamp;
	uint8_t code;
	char name[nameMax];
} RemovedUser;

typedef struct __attribute__((packed))
{
	uint8_t type;
	uint16_t length;	
} Header;

typedef struct __attribute__((packed))
{
	LoginRequest loginRequest;
	LoginResponse loginResponse;
	ClientToServer clientToServer;
	ServerToClient serverToClient;
	AddedUser addedUser;
	RemovedUser removedUser;
} Body;

typedef struct __attribute__((packed))
{
	Header header;
	Body body;
} Message;


int networkReceive(int fd, Message *buffer);
int networkSend(int fd, const Message *buffer);
void createMessage(Message *messageBuffer, const char *textBuffer);
void setLength(Message *messageBuffer, int contentLength);
void convertMessageToNetworkOrder(Message *messageBuffer);
Message initMessage(uint8_t type);

#endif
