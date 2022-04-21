#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <arpa/inet.h>

#define MAXPENDING 10
#define RCVBUFSIZE 512

void handle_err(const char *msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

int main(void) {
	int servSock;
	int clientSock;
	struct sockaddr_in echoServAddr;
	struct sockaddr_in echoClntAddr;
	socklen_t clntLen;
	socklen_t rcvMsgSize;
	char echoBuffer[RCVBUFSIZE];
	
	if ((servSock = socket(AF_INET, SOCK_STREAM, 0)) < 0) 
		handle_err("socket()");

	echoServAddr.sin_family = AF_INET;
	echoServAddr.sin_port = htons(5100);
	echoServAddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
	if (bind(servSock, (struct sockaddr *) &echoServAddr, sizeof(struct sockaddr_in)) < 0) 
		handle_err("bind()");

	if (listen(servSock, MAXPENDING) < 0) handle_err("listen()");


	while (1) {
		clntLen = sizeof(echoClntAddr);
		printf("Awaiting connection...\n");
		if ((clientSock = accept(servSock, (struct sockaddr *) &echoClntAddr, &clntLen)) < 0)
			handle_err("accept()");

		printf("Connected, reading response\n");

		if ((rcvMsgSize = recv(clientSock, echoBuffer, RCVBUFSIZE, 0)) < 0)
			handle_err("recv()");

		printf("Read ''%s'\nSending back\n", echoBuffer);

		while (rcvMsgSize > 0) {
			if (send(clientSock, echoBuffer, rcvMsgSize, 0) != rcvMsgSize)
				handle_err("send()");
			if ((rcvMsgSize = recv(clientSock, echoBuffer, RCVBUFSIZE, 0)) < 0)
				handle_err("recv()");
		}
	}
}
