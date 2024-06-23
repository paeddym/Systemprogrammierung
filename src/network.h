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
	char name[32];
} LoginRequest;

typedef struct __attribute__((packed))
{
	uint32_t magic;
	uint8_t code;
	char name[32];
} LoginResponse;

typedef struct __attribute__((packed))
{
	char text[512];
} ClientToServer;

typedef struct __attribute__((packed))
{
	uint64_t timestamp;
	char sender[32];
	char text[512];
} ServerToClient;

typedef struct __attribute__((packed))
{
	uint64_t timestamp;
	char name[32];
} AddedUser;

typedef struct __attribute__((packed))
{
	uint64_t timestamp;
	uint8_t type;
	char name[32];
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

#endif
