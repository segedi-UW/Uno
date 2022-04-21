#ifndef ONLINE_H
#define ONLINE_H

#define ONLINE_BUFFER 512

enum PCmd {
	TERMINATE, CONNECT
};

struct Packet {
	enum PCmd cmd;
};

#endif
