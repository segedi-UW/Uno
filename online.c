/*
 * Simple tcp ran service that allows online 
 * capabilities for an application. 
 *
 * The handle() function pointer parameter for the hostOnline() function
 * allows an Application to react to the data and the
 * new connections. It is called each time a successful
 * accept() occurs. It is perfectly safe to send NULL if you
 * have no need for the handle() method.
 *
 * The protocol handles working with sockets directly.
 * Data is transfered via a struct Packet defined as such:
 *
 * struct Packet {
 * 	  int cmd;
 * 	  char data[]
 * }
 *
 * The size of the data field is contained by PACKET_DATAN.
 * It is a maximum of 252 on most systems, although may vary
 * to 248.
 *
 * uniOnline(), multiOnline(), and rcvOnline() are blocking
 * by default. They return -1 if there is a disconnect or other
 * failure, and may print out an error with perror(). errno is
 * not reset.
 *
 * author: Anthony Segedi
 */

#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <netinet/ip.h>
#include <time.h>
#include <sys/select.h>
#include <sys/time.h>
#include <sys/types.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>
#include <stdbool.h>
#include <errno.h>

#include "online.h"
#include "agLinkedList.h"
#include "agLLIterator.h"
#include "agAnsi.h"

#define START_PORT 3543
// 1024 + 1 is outside reserved space
#define UNRESERVED 1025 

int servSock;
struct sockaddr_in servAddr;
char buffer[sizeof(struct Packet)];
agLinkedList *lost;

void handleError(const char *err) {
	perror(err);
	exit(EXIT_FAILURE);
}

static void initOnline(void) {
	lost = agLLInit();
}

int hostOnline(int maxN, agLinkedList *connections, void (*handle)()) {
	int bindStatus;
	int port = START_PORT;
	int maxfd = 1;
	int ready = -1; // set to non zero to not print timeout msg
	fd_set rfds;
	fd_set wfds;
	fd_set efds;
	struct timeval t;

	initOnline();

	srand(time(NULL));

	// create socket
	if ((servSock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		handleError("socket()");

	servAddr.sin_family = AF_INET;
	servAddr.sin_port = htons(port);
	servAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	// bind port
	while ((bindStatus = bind(servSock, (struct sockaddr *) &servAddr, sizeof(struct sockaddr_in))) == -1) {
		port = (rand() % 3000) + UNRESERVED; 
		servAddr.sin_port = htons(port);
	}
	if (bindStatus < 0) return -1;

	if (listen(servSock, maxN) < 0) handleError("listen()");

	size_t bufn = 32;
	char *buf = malloc(bufn);
	if (buf == NULL) handleError("malloc()");
	socklen_t acptlen;
	struct Packet p;

	printf(
			"Hosting from "ANSI2(ANSI_FG_CYAN, "port %d") "\n"
			ANSI2(ANSI_FG_GREEN, "Connected:\n")
			"Press " ANSI2(ANSI_FG_YELLOW, "<ENTER>") " to stop accepting players"
			, ntohs(servAddr.sin_port));
	fflush(stdout);
	maxfd = servSock + 1;

	struct Connection c;
	agLLIterator *cit = agLLIInit(connections);

	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	FD_ZERO(&efds);

	do  {
		if (ready > 0) {
			if (FD_ISSET(fileno(stdin), &rfds)) {
				getline(&buf, &bufn, stdin);
				break;
			} 
			if (connections->size < maxN && FD_ISSET(servSock, &rfds)) {
				// accept
				acptlen = sizeof(struct sockaddr_in);
				if ((c.sock = accept(servSock, (struct sockaddr *) &(c.addr), &acptlen)) >= 0) {
					c.id = connections->size;
					c.len = acptlen;
					if (c.sock >= maxfd - 1) maxfd = c.sock + 1;
					if (rcvOnline(&c, &p) <= 0) handleError("rcv()");
					printf(
						ANSI(ANSI_ERASE_LN_ALL) ANSI_CURSOR_COL_ABS("0") ANSI2(ANSI_FG_CYAN, "%s") " has joined the game!\n"
						"Press " ANSI2(ANSI_FG_YELLOW, "<ENTER>") " to stop accepting players"
							, p.data);
					fflush(stdout);
					agLLPush(connections, &c, sizeof(c));
					if (handle != NULL)
						handle();
				}
				else perror("Failed to accept connection\n");
				ready--;
			}
			for (int i = 0; ready > 0 && i < connections->size; i++) {
				if (FD_ISSET(c.sock, &efds)) {
					printf("Error with fd(%d)!", c.sock);
					perror("efds");
					ready--;
				}
			}
		}

		FD_SET(servSock, &rfds);
		FD_SET(fileno(stdin), &rfds); // for user input
		agLLIReset(cit);
		while (agLLIHasNext(cit)) {
			c = *(struct Connection*)agLLINext(cit);
			FD_SET(c.sock, &efds);
		}

		t.tv_sec = 30;
		t.tv_usec = 0;
	} while ((ready = select(maxfd, &rfds, &wfds, &efds, &t)) >= 0);

	agLLIFree(cit);
	free(buf);
	return connections->size;
}



int joinOnline(agLinkedList *clist, char *ip, int port) {
	struct Connection c;

	initOnline();

	printf("Attempting to join %s on port %d\n", ip, port);
	if ((c.sock = socket(AF_INET, SOCK_STREAM, 0)) < 0)
		handleError("socket()");

	c.addr.sin_family = AF_INET;
	c.addr.sin_addr.s_addr = htonl(INADDR_ANY);
	c.addr.sin_port = htons(port);

	if (connect(c.sock, (struct sockaddr *) &(c.addr), sizeof(struct sockaddr_in)))
		handleError("connect()");

	agLLPush(clist, &c, sizeof(struct Connection));
	return 0;
}

int multiOnline(agLinkedList *clist, struct Packet *p) {
	if (!agLLIsEmpty(lost)) agLLClear(lost); // reset list
	agLLIterator *it = agLLIInit(clist);
	struct Connection *c;
	int l = 0;
	while (agLLIHasNext(it)) {
		c = agLLINext(it);
		if (uniOnline(c, p) <= 0) {
			l++;
			agLLPush(lost, c, sizeof(struct Connection));
		}
	}
	agLLIFree(it);
	return l;
}

agLinkedList *multiLost(void) {
	return lost;
}

int uniOnline(const struct Connection *c, struct Packet *p) {
	size_t psize = sizeof(buffer);
	int sent = 0;
	int tsent = 0;
	memcpy(buffer, p, psize);
	while (tsent < psize) {
		sent = send(c->sock, buffer + tsent, psize - tsent, MSG_NOSIGNAL);
		if (sent <= 0) return sent;
		tsent += sent;
	}
	return tsent;
}



int rcvOnline(const struct Connection *c, struct Packet *p) {
	size_t psize = sizeof(buffer);
	int rcv = 0;
	int trcv = 0;
	while (trcv < psize) {
		rcv = recv(c->sock, buffer + trcv, psize - trcv, 0);
		if (rcv <= 0) return rcv;
		trcv += rcv;
	}
	memcpy(p, buffer, psize);
	return trcv;
}

void closeOnline(agLinkedList *clist) {
	if (servSock > 0) {
		printf("Closing servSock\n");
		struct linger lo = { 1, 0 };
		setsockopt(servSock, SOL_SOCKET, SO_LINGER, &lo, sizeof(lo));
		shutdown(servSock, SHUT_RDWR);
		close(servSock);
	}

	if (lost != NULL)
		agLLFree(lost);

	agLLIterator *it = agLLIInit(clist);
	struct Connection *c;
	while (agLLIHasNext(it)) {
		c = agLLINext(it);
		close(c->sock);
	}
	agLLIFree(it);
}
