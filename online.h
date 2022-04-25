#ifndef ONLINE_H
#define ONLINE_H
#include <netinet/in.h>
#include "agLinkedList.h"

#define PACKET_DATAN (1024 - sizeof(enum Command))

enum Command {
	DISPLAY, RESPONSE, EXIT
};

struct Connection {
	int id;
	int sock;
	struct sockaddr_in addr;
	socklen_t len;
};

struct Packet {
	int cmd;
	char data[PACKET_DATAN];
};

int hostOnline(int maxN, agLinkedList *c, int port, void (*handle)());
int joinOnline(agLinkedList *clist, char *ip, int port);
int multiOnline(agLinkedList *clist, struct Packet *p);
agLinkedList *multiLost(void);
int uniOnline(const struct Connection *c, struct Packet *p);
int rcvOnline(const struct Connection *c, struct Packet *p);
void closeOnline(agLinkedList *clist);
int isConnected(const struct Connection *c);

#endif
