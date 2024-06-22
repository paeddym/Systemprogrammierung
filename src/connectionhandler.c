#include <errno.h>
#include "connectionhandler.h"
#include "util.h"
#include "user.h"
#include "clientthread.h"

static int createPassiveSocket(in_port_t port)
{
	int fd = -1;
	
	fd = socket(AF_INET, SOCK_STREAM, 0);
	if (fd == -1) {
		errno = EACCES;
		errorPrint("Failed to create socket!");
		return -1;
	}

	const int active = 1;
	setsockopt(fd, SOL_SOCKET, SO_REUSEADDR, &active, sizeof(active));

	struct sockaddr_in server_address;
	memset(&server_address, 0, sizeof(server_address));
	server_address.sin_addr.s_addr = htonl(INADDR_ANY);
	server_address.sin_port = htons(port);
	server_address.sin_family = AF_INET;

	if(bind(fd, (struct sockaddr *)&server_address, sizeof(server_address)) == -1) {
		errorPrint("Failed to bind!");
		return -1;
	}
	if(listen(fd, 5) == -1) {
		errorPrint("Failed to listen");
		return -1;
	}
	
	return fd;
}

int connectionHandler(in_port_t port)
{
	const int fd = createPassiveSocket(port);
	if(fd == -1) {
		errorPrint("Unable to create server socket");
		return -1;
	}

	if(initMutex != 0) {
		errorPrint("Failed to initialize mutex!");
		return -1;
	}

	for(;;) {
		int client_socket = accept(fd, NULL, NULL);
		
		if(client_socket == -1) {
			errorPrint("Failed to accept connection!");
			return -1;
		}

		lockUser();
		User *myUser = allocateSpaceForUser();
		myUser->thread = 0;
		myUser->name[0] = '\0';
		myUser->sock = client_socket;

		int status = pthread_create(&myUser->thread, NULL, clientthread, myUser);
		if(status != 0){
			errorPrint("Failed to create thread!");
			return 0;
		}
	}

	return 0;	//never reached
}
