#include <stdlib.h>
#include <stdio.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <string.h>

void handle_err(const char *msg) {
	perror(msg);
	exit(EXIT_FAILURE);
}

int main(void) {
	int clientSock;
	struct sockaddr_in echoServeAddr;
	const char *echoString = "Hey there dumbass\n";
	const int echoStringLen = strlen(echoString);

	if ((clientSock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		handle_err("socket()");

	printf("socket created #%d\n", clientSock);

	echoServeAddr.sin_family = AF_INET;
	echoServeAddr.sin_addr.s_addr = /*inet_addr("192.168.1.222"); */ htonl(INADDR_LOOPBACK);
	echoServeAddr.sin_port = htons(5100);

	if (connect(clientSock, (struct sockaddr *) &echoServeAddr, sizeof(echoServeAddr)) < 0)
		handle_err("connect()");


	if (send(clientSock, echoString, echoStringLen, 0) != echoStringLen)
		handle_err("send()");

}
