#include <stdio.h>
#include "connectionhandler.h"
#include "util.h"
#include "broadcastagent.h"
#include <getopt.h>
#include <string.h>
#include <signal.h>
#include <stdlib.h>

static void sigint_handler() {
    infoPrint("Caught Signal 2 SIGINT, shutting down!");
    broadcastAgentCleanup();
    exit(EXIT_SUCCESS);
}

static void sigsegv_handler() {
    errorPrint("Caught Segmentation Fault SIGSEGV, shutting down!");
    broadcastAgentCleanup();
    exit(EXIT_FAILURE);
}

static void sigpipe_handler() {
    errorPrint("Caught Broken Pipe Error SIGPIPE, shutting down!");
}


int main(int argc, char **argv)
{
	utilInit(argv[0]);
	infoPrint("Chat server, group 13");
	const char *optionsFormat = "dm:p:h";
	int optionsIndex = 0;
	in_port_t port = 8111;
	char *stringPort = NULL;
	while ((optionsIndex = getopt(argc, argv, optionsFormat)) != -1) {
        switch (optionsIndex) {
            case 'd':
                if (debugEnabled() == 0){
                    debugEnable();
                }
                break;
            case 'm':
                if (styleEnabled() == 1){
                    styleDisable();
                }
                break;
            case 'p':
                stringPort = strdup(optarg);
                uint16_t strLength = strlen(stringPort);
                for (int i = 0; i < strLength; i++) {
                    if (stringPort[i] < 0x30 || stringPort[i] > 0x39) {
                        errorPrint("Invalid port number!");
                        exit(EXIT_FAILURE);
                    }
                }
                port = (uint16_t) atoi(stringPort);
                if (port < 1023) {
                    errorPrint("Invalid port number!");
                    exit(EXIT_FAILURE);
                }
                break;
            case 'h':
				exit(EXIT_SUCCESS);
            default:
                errorPrint("Option %c incorrect", optionsIndex);
                exit(EXIT_FAILURE);
        }
    }

	if (broadcastAgentInit() == -1) {
		printf("broadcastAgentInit failed\n");
        return EXIT_FAILURE;
    }

    sigset_t mask_sigint;
    sigset_t mask_sigsegv;
    sigset_t mask_sigpipe;
    sigemptyset(&mask_sigint);
    sigemptyset(&mask_sigsegv);
    sigemptyset(&mask_sigpipe);

    const struct sigaction action_sigint = {
        .sa_handler = sigint_handler,
        .sa_mask = mask_sigint, 
        .sa_flags = 0
    };

    const struct sigaction action_sigsegv = {
        .sa_handler = sigsegv_handler,
        .sa_mask = mask_sigsegv,  
        .sa_flags = 0
    };

    const struct sigaction action_sigpipe = {
        .sa_handler = sigpipe_handler,
        .sa_mask = mask_sigpipe,
        .sa_flags = 0
    };

    if (sigaction(SIGINT, &action_sigint, NULL) != 0) {
        errorPrint("Cannot register signal handler!");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGSEGV, &action_sigsegv, NULL) != 0) {
        errorPrint("Cannot register signal handler!");
        exit(EXIT_FAILURE);
    }
    if (sigaction(SIGPIPE, &action_sigpipe, NULL) != 0) {
        errorPrint("Cannot register signal handler!");
        exit(EXIT_FAILURE);
    }

	const int result = connectionHandler((in_port_t)port);
	printf("connectionHandler failed %d\n", result);

	return result != -1 ? EXIT_SUCCESS : EXIT_FAILURE;
}
