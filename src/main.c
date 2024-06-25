#include <stdlib.h>
#include "connectionhandler.h"
#include "util.h"
#include "broadcastagent.h"
#include <stdio.h>
#include <signal.h>

static void sigint_handler(){
	broadcastAgentCleanup();
	exit(EXIT_SUCCESS);
}

static void sigsegv_handler(){
	broadcastAgentCleanup();
	exit(EXIT_FAILURE);
}

static void sigpipe_handler(){
	broadcastAgentCleanup();
	exit(EXIT_FAILURE);
}

int main(int argc, char **argv)
{
	utilInit(argv[0]);
	infoPrint("Chat server, group 01");

	//TODO: evaluate command line arguments


	//TODO: perform initialization
	if (broadcastAgentInit() == -1) {
		printf("Failed to initialize broadcast agent!\n");
		return EXIT_FAILURE;
	}

	sigset_t mask_sigint;
	sigset_t mask_sigsegv;
	sigset_t mask_sigpipe;
	sigemptyset(&mask_sigint);
	sigemptyset(&mask_sigsegv);
	sigemptyset(&mask_sigpipe);

	const struct sigaction sa_sigint = {
		.sa_handler = sigint_handler,
		.sa_mask = mask_sigint,
		.sa_flags = 0
	};

	const struct sigaction sa_sigsegv = {
		.sa_handler = sigsegv_handler,
		.sa_mask = mask_sigsegv,
		.sa_flags = 0
	};

	const struct sigaction sa_sigpipe = {
		.sa_handler = sigpipe_handler,
		.sa_mask = mask_sigpipe,
		.sa_flags = 0
	};

	if (sigaction(SIGINT, &sa_sigint, NULL) != 0) {
		perror("sigaction");
		return EXIT_FAILURE;
	}

	if (sigaction(SIGSEGV, &sa_sigsegv, NULL) != 0) {
		perror("sigaction");
		return EXIT_FAILURE;
	}

	if (sigaction(SIGPIPE, &sa_sigpipe, NULL) != 0) {
		perror("sigaction");
		return EXIT_FAILURE;
	}

	//TODO: use port specified via command line
	const int result = connectionHandler((in_port_t)8111);

	//TODO: perform cleanup, if required by your implementation
	return result != -1 ? EXIT_SUCCESS : EXIT_FAILURE;
}
