#include <unistd.h>
#include <stdlib.h>
#include <stdio.h>
#include <stdbool.h>
#include <string.h>

#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/ip.h>


void handle_error(const char *err) {
	perror(err); 
	exit(EXIT_FAILURE);
}

int main(int argc, char *argv[]) {
	extern char *optarg;
	extern int optind;
	char *optstr = "p:";
	int optret;
	int port = -1;

	while ((optret = getopt(argc, argv, optstr)) != -1) {
		switch (optret) {
			case 'p':
				port = atoi(optarg);
				if (port < 0) {
					fprintf(stderr, "Entered port is invalid: %d\n", port);
					return EXIT_FAILURE;
				}
				break;
			default:
				printf("Unrecognized option: %c\n", optret);
		}
	}

	if (port < 0) {
		fprintf(stderr, "Client requires port specified with -p option\n");
		return EXIT_FAILURE;
	}

	int c = socket(AF_INET, SOCK_STREAM, 0);
	if (c < 0) handle_error("Could not init socket");

	// internet sockaddr
	struct sockaddr_in caddr;
	memset(&caddr, 0, sizeof (struct sockaddr));
	caddr.sin_family = AF_INET;
	caddr.sin_port = htons(port);
	caddr.sin_addr.s_addr = /*htonl(INADDR_ANY);*/ htonl(INADDR_LOOPBACK);

	socklen_t caddrlen = sizeof(struct sockaddr_in);

	if (connect(c, (struct sockaddr *) &caddr, caddrlen) != 0) handle_error("Failed to connect");

	printf("Connected successfully\n");

	printf("Closing connection\n");
	close(c);

	return EXIT_SUCCESS;
}
