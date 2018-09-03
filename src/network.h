#ifndef CHAT_PROTOCOL_H
#define CHAT_PROTOCOL_H

#include <stdint.h>

enum { MSG_MAX = 1024 };

typedef struct __attribute__((packed))
{
	uint16_t len;		//real length of the text (big endian, len <= MSG_MAX)
	char text[MSG_MAX];	//text message
} Message;

int networkReceive(int fd, Message *buffer);
int networkSend(int fd, const Message *buffer);

#endif
