#ifndef CHAT_PROTOCOL_H
#define CHAT_PROTOCOL_H

#include <stdint.h>

enum ERROR_CODES {
    clientClosedConnectionError = 1,
    error = 2,
    noError = 3,
};

enum MESSAGE_TYPES {
    loginRequestType = 0,
    loginResponseType = 1,
    clientToServerType = 2,
    serverToClientType = 3,
    userAddedType = 4,
    userRemovedType = 5
};

enum MESSAGE_LIMITS {
    nameMin = 1,
    nameMax = 32,
    originalSenderMax = 32,
    textMax = 512,
    headerMax = 3,
    bodyMax = 552,
    messageMax = headerMax + bodyMax + 1
};

enum LOGINRESPONSE_CODES {
    success = 0,
    nameAlreadyTaken = 1,
    nameInvalid = 2, 
    protocolVersionMismatch = 3,
    otherServerError = 255
};

enum USERREMOVED_CODES {
    connectionClosedByClient = 0,
    kickedFromTheServer = 1, 
    communicationError = 2
};

typedef struct __attribute__((packed))
{
    uint8_t type;
    uint16_t length;
} Header;

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
    char sName[nameMax];
} LoginResponse;

typedef struct __attribute__((packed))
{
    char text[textMax];
} ClientToServer;

typedef struct __attribute__((packed))
{
    uint64_t timestamp;
    char originalSender[originalSenderMax];
    char text[textMax];
} ServerToClient;

typedef struct __attribute__((packed))
{
    uint64_t timestamp;
    char name[nameMax];
} UserAdded;

typedef struct __attribute__((packed))
{
    uint64_t timestamp;
    uint8_t code;
    char name[nameMax];
} UserRemoved;

typedef union __attribute__((packed))
{
    LoginRequest loginRequest;
    LoginResponse loginResponse;
    ClientToServer clientToServer;
    ServerToClient serverToClient;
    UserAdded userAdded;
    UserRemoved userRemoved;
} Body;

typedef struct __attribute__((packed))
{
	Header header;	
	Body body;	
} Message;

int networkReceive(int fd, Message *buffer);
int networkSend(int fd, const Message *buffer);
void setMessageLength(Message *buffer, int stringLength);
void createMessage(Message *buffer, const char *textBuffer);
Message initMessage(uint8_t type);
void prepareMessage(Message *buffer);

#endif

