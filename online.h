#ifndef ONLINE_H
#define ONLINE_H

#define PACKET_DATAN (256 - sizeof(enum Command))

#include "agLinkedList.h"

#if defined(_WIN32) || defined(__CYGWIN)
#include <winsock2.h>

#elif  defined(__linux__)
#include <netinet/in.h>
#endif


struct Connection {
	int id;
	int sock;
	struct sockaddr_in addr;
	long len;
};

enum Command {
	DISPLAY, RESPONSE, EXIT
};

struct Packet {
	int cmd;
	char data[PACKET_DATAN];
};

int hostOnline(int maxN, agLinkedList *c, void (*handle)());
int joinOnline(agLinkedList *clist, char *ip, int port);
int multiOnline(agLinkedList *clist, struct Packet *p);
agLinkedList *multiLost(void);
int uniOnline(const struct Connection *c, struct Packet *p);
int rcvOnline(const struct Connection *c, struct Packet *p);
void closeOnline(agLinkedList *clist);
int isConnected(const struct Connection *c);

#endif
