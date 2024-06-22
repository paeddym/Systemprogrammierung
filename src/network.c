#include <errno.h>
#include "network.h"
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>

int networkReceive(int fd, Message *buffer)
{
	ssize_t receivedBytes = recv(fd, &buffer->header, sizeof(buffer->header), MSG_WAITALL);
	//TODO: Convert length byte order
	//TODO: Validate length
	//TODO: Receive text

	return -1;
}

int networkSend(int fd, const Message *buffer)
{
	//TODO: Send complete message

	errno = ENOSYS;
	return -1;
}