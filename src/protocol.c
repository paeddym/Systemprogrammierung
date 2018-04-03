#include <errno.h>
#include "protocol.h"

int networkReceive(Message *buffer)
{
	//TODO: Receive length
	//TODO: Convert length byte order
	//TODO: Validate length
	//TODO: Receive text

	errno = ENOSYS;
	return -1;
}

int networkSend(const Message *buffer)
{
	//TODO: Send complete message

	errno = ENOSYS;
	return -1;
}
