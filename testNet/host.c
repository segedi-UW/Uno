#include <stdlib.h>
#include <stdbool.h>
#include <stdio.h>
#include <string.h>
#include <time.h>
#include <unistd.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <netinet/tcp.h>
#include <sys/types.h> 
#include <sys/select.h>
#include <pthread.h>

#include "online.h"

#define BACKLOG 10
#define CONNECTIONS 11
#define TIMEOUT 10000
#define START_PORT 3543
#define PORT_BIND_ATTEMPTN 20

// Create socket with socket()
//To recieve incoming:
//1 bind() socket to local addr and port
//2 listen() to put socket in listen mode
//3 accept() to accept the connection

int max_fd;
pthread_mutex_t mutex;
pthread_t threads[4];


void handleError(const char *msg) {
	perror(msg);
	exit(1);
}

void tryBind(int sockfd, struct sockaddr_in *addr) {

	// have a set port to try to keep consistent
	int port = START_PORT;
	int status = -1;
	srand(time(NULL));

	addr->sin_port = htons(port);

	for (int i = 0; (status = bind(sockfd, (struct sockaddr *) addr, sizeof(struct sockaddr_in))) == -1 && i < PORT_BIND_ATTEMPTN; i++) {
		port = (rand() % 3000) + 1024;
		addr->sin_port = htons(port);
	}

	if (status < 0) handleError("bind()");
}

static void *acceptConnections(void *arg) {
	
	
	return NULL;
}

int main(void) {
	int hsock, csock[10];
	int cind = 0;
	int status;
	char pbuf[ONLINE_BUFFER];
	struct sockaddr_in addr;
	struct Packet inp;
	struct Packet outp;


	memset(&addr, 0, sizeof(struct sockaddr_in));
	addr.sin_family = AF_INET;
	addr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	socklen_t addrlen;

	hsock = socket(AF_INET, SOCK_STREAM, 0);
	if (hsock == -1) handleError("host socket fd");

	tryBind(hsock, &addr);

	if ((status = listen(hsock, BACKLOG)) == -1) handleError("Failed to host");

	printf("Listening on %u\n", ntohs(addr.sin_port));

	addrlen = sizeof(struct sockaddr_in);

	// setup thread safety
	pthread_mutex_init(&mutex, NULL);
	// start accept thread
	if (pthread_create(threads, NULL, &acceptConnections, NULL) != 0)

	return 0;
}

