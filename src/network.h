#ifndef CHAT_PROTOCOL_H
#define CHAT_PROTOCOL_H

#include <stdint.h>

/* TODO: When implementing the fully-featured network protocol (including
 * login), replace this with message structures derived from the network
 * protocol (RFC) as found in the moodle course. */
enum { MSG_MAX = 1024 };

enum MSG_TYPE{
	MSG_LOGIN_REQUEST = 0,
	MSG_LOGIN_RESPONSE = 1,
	MSG_CLIENT_TO_SERVER = 2,
	MSG_SERVER_TO_CLIENT = 3,
	MSG_ADDED_USER = 4,
	MSG_REMOVED_USER = 5
};

typedef struct __attribute__((packed))
{
	uint32_t magic;
	uint8_t version;
	char requestName[nameMax];
} LoginRequest;

typedef struct __attribute__((packed))
{
	uint32_t magic;
	uint8_t code;
	char responseName[nameMax];
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

enum MESSAGE_LIMITS {
    nameMin = 1,
    nameMax = 31,
    textMax = 512,
	senderMax = 32
};

enum MESSAGE_TYPES {
    loginRequestType = 0,
    loginResponseType = 1,
    client2ServerType = 2,
    server2ClientType = 3,
    userAddedType = 4,
    userRemovedType = 5
};

enum ERROR_CODES {
    noError = 0,
    clientClosedConnectionError = 1,
    error = -1
};

int networkReceive(int fd, Message *buffer);
int networkSend(int fd, const Message *buffer);

Message initMessage(uint8_t type);

#endif
