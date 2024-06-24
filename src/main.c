#include <stdlib.h>
#include "connectionhandler.h"
#include "util.h"

int main(int argc, char **argv)
{
	utilInit(argv[0]);
	infoPrint("Chat server, group 01");

	//TODO: evaluate command line arguments
	//TODO: perform initialization

	//TODO: use port specified via command line
	const int result = connectionHandler((in_port_t)8111);

	//TODO: perform cleanup, if required by your implementation
	return result != -1 ? EXIT_SUCCESS : EXIT_FAILURE;
}
