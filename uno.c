#include <stdarg.h>
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <errno.h>
#include <time.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <stdarg.h>
#include <signal.h>
#include "cards.h"
#include "agLinkedList.h"
#include "agLLIterator.h"
#include "agString.h"
#include "online.h"
#include "agAnsi.h"

#define PLAY_ON_DRAW true
#define NO_PLAY_ON_DRAW false
#define PLAYERN 10

typedef struct UnoPlayer {
	unsigned char id;
	unsigned char isBot;
	char name[UNO_PLAYER_NAMEN];
	agLinkedList *deck;
	int score;
} UnoPlayer;

static void display(const void *f, ...);

static char *botNames[] = {"carl_bot", "gwen_bot", "jason_bot", "LANAAA_bot", "chief_bot", "cop_bot","monkey_bot", "destroyer_bot", "ancient_bot"};
static agLinkedList *deck, *discard;
static char isBotDelay = true;
static int player_id = 0;
static agLinkedList *connections;
static agLinkedList *players;

static void handleLost(UnoPlayer *p) {
	display("Lost connection to %s!\n", p->name);
	agLLIterator *it = agLLIInit(players);
	UnoPlayer *ip;
	int size = players->size;
	int pid;
	while (agLLIHasNext(it)) {
		ip = agLLINext(it);
		if (ip->id == p->id) {
			pid = ip->id;
			agLLIRemoveFree(it);
			break;
		}
	}
	agLLIFree(it);

	agLLIterator *cit = agLLIInit(connections);
	struct Connection *c;
	while (agLLIHasNext(cit)) {
		c = agLLINext(cit);
		if (c->id == pid) {
			agLLIRemoveFree(cit);
			break;
		}
	}
	agLLIFree(cit);

	if (size <= players->size) {
		fprintf(stderr, "Failed to remove disconnected player\n");
		exit(EXIT_FAILURE);
	}

	if (players->size <= 1) {
		printf("Game can no longer continue due to lack of players.\n");
		exit(EXIT_SUCCESS);
	}
}

static void multiHandle(struct Packet *p) {
	if (multiOnline(connections, p) < 0) {
		agLinkedList *lost = multiLost();
		agLLIterator *it, *cit; 
		cit = agLLIInit(lost);
		it = agLLIInit(players);
		struct Connection *c, *rc;
		UnoPlayer *player;
		while (agLLIHasNext(cit)) {
			c = agLLINext(cit);
			rc = agLLRemovep(connections, c, sizeof(struct Connection));
			while (agLLIHasNext(it)) {
				player = agLLINext(it);
				if (player->id == rc->id) {
					agLLIRemoveFree(it);
					agLLIRemoveFree(cit);
				}
			}
		}
		agLLIFree(it);
		agLLIFree(cit);
	}
}

static void rcvHandle(UnoPlayer *p, struct Connection *c, struct Packet *packet) {
	if (rcvOnline(c, packet) <= 0)
		handleLost(p);
}

static void uniHandle(UnoPlayer *p, struct Connection *c, struct Packet *packet) {
	if (uniOnline(c, packet) <= 0)
		handleLost(p);
}

static void display(const void *f, ...) {
	struct Packet p;
	p.cmd = DISPLAY;

	va_list ap;
	va_start(ap, f);

	vsnprintf(p.data, sizeof(p.data), f, ap);
	if (!agLLIsEmpty(connections))
		multiHandle(&p);

	va_start(ap, f);
	vprintf(f, ap);
}

static void displayPlayer(UnoPlayer *p, const void *f, ...) {
	va_list ap;
	va_start(ap, f);

	agLLIterator *it = agLLIInit(connections);
	int contains = false;
	struct Connection *c;

	while (agLLIHasNext(it)) {
		c = agLLINext(it);
		if (c->id == p->id) {
			contains = true;
			break;
		}
	}
	if (!contains) {
		vprintf(f, ap);
		agLLIFree(it);
		return;
	}

	struct Packet packet;
	packet.cmd = DISPLAY;

	agLLIReset(it);
	while (agLLIHasNext(it)) {
		c = agLLINext(it);
		if (c->id == p->id) {
			vsnprintf(packet.data, sizeof(packet.data), f, ap);
			uniHandle(p, c, &packet);
			agLLIFree(it);
			return;
		}
	}

	agLLIFree(it);
}

static void interpret(const struct Packet *p) {
	switch (p->cmd) {
		case EXIT:
			printf(ANSI2(ANSI_FG_RED, "\nHost has ended the game\n"));
			exit(EXIT_SUCCESS);
			break;
		case RESPONSE:
			fprintf(stderr, ANSI2(ANSI_FG_RED, "Handling response outside of local code!\n"));
			break;
		case DISPLAY:
			printf("%s", p->data);
			fflush(stdout);
			break;
	}
}

static void handleLostClient(void) {
	perror("Lost connection to host!");
	closeOnline(connections);
	exit(EXIT_FAILURE);
}

static void playOnline(char *name) {
	struct Connection *c = agLLGet(connections, 0);
	struct Packet p;
	memset(&p, 0, sizeof(p));

	printf("Sending init info...\n");

	p.cmd = RESPONSE;
	agStrntcpy(p.data, name, sizeof(p.data));
	if ((uniOnline(c, &p)) <= 0) handleLostClient();
	if ((uniOnline(c, &p)) <= 0) handleLostClient();

	printf(
			ANSI2(ANSI_FG_BLUE, "%s") ", you have joined the game successfully!\n"
			"Waiting for game start...\n"
			, name);

	struct timeval t;
	int maxfd = c->sock + 1;
	fd_set rfds, wfds, efds;
	size_t bufn = 512;
	char *buf = malloc(bufn);
	if (buf == NULL) {
		perror("malloc()");
		exit(EXIT_FAILURE);
	}
	int ready = -1, isHostReading = 0;

	FD_ZERO(&rfds);
	FD_ZERO(&wfds);
	FD_ZERO(&efds);

	do {
		if (ready > 0) {
			if (FD_ISSET(c->sock, &efds)) {
				free(buf);
				perror("Socket error detected!");
				return;
			}
			if (FD_ISSET(c->sock, &rfds)) {
				if (rcvOnline(c, &p) <= 0) handleLostClient();
				interpret(&p);
				continue; // continues to read if needed
				// reads have higher priority than writes
			}

			if (FD_ISSET(c->sock, &wfds)) {
				isHostReading = true;
				FD_CLR(c->sock, &wfds);
			}

			if (FD_ISSET(fileno(stdin), &rfds)) {
				getline(&buf, &bufn, stdin);
				if (isHostReading) {
					isHostReading = false;
					// only send input when wanted
					p.cmd = RESPONSE;
					agStrntcpy(p.data, buf, bufn);
					if ((uniOnline(c, &p)) <= 0) handleLostClient();
				}
			}
		}

		FD_SET(c->sock, &rfds);
		if (!isHostReading)
			FD_SET(c->sock, &wfds);
		FD_SET(c->sock, &efds);
		FD_SET(fileno(stdin), &rfds); // for user input

		t.tv_sec = 5;
		t.tv_usec = 0;
	} while ((ready = select(maxfd, &rfds, &wfds, &efds, &t)) >= 0);

	free(buf);
}

static void scorePlayers() {
	agLLIterator *pit, *it;
	pit = agLLIInit(players);
	UnoCard *c;
	UnoPlayer *p;
	while (agLLIHasNext(pit)) {
		p = agLLINext(pit);
		it = agLLIInit(p->deck);
		while (agLLIHasNext(it)) {
			c = agLLINext(it);
			if (c->number < 10)
				p->score += c->number;
			else if (c->number < WILD + 9)
				p->score += 20;
			else
				p->score += 50;
		}
	}
	agLLIFree(pit);
	agLLIFree(it);
}

// uses insertSort - assumes a presorted deck!
static void insertCard(UnoPlayer *p, UnoCard *c) {
	agLLIterator *it = agLLIInit(p->deck);
	UnoCard *cmp;
	int cmpval, cval;
	while (agLLIHasNext(it)) {
		cmp = agLLINext(it);
		cmpval = cmp->number + 20*cmp->color;
		cval = c->number + 20 * c->color;
		if (cval < cmpval) {
			agLLIInsert(it, c, sizeof(UnoCard));
			return;
		}
	}
	agLLIFree(it);
	agLLPush(p->deck, c, sizeof(UnoCard));
}

static UnoPlayer *initUnoPlayer(const char *name, const unsigned char isBot) {
	UnoPlayer *p = malloc(sizeof(UnoPlayer));
	if (p == NULL) {
		perror("malloc()");
		exit(EXIT_FAILURE);
	}
	p->id = player_id++;
	p->isBot = isBot;
	agStrntcpy(p->name, name, sizeof(p->name));
	p->deck = agLLInit();
	p->score = 0;
	return p;
}

static void deal() {
	UnoCard *card;
	agLLIterator *it = agLLIInit(players);
	UnoPlayer *p;
	while (agLLIHasNext(it)) {
		p = agLLINext(it);
		for (int c = 0; c < 7; c++) {
			card = agLLPop(deck);
			insertCard(p, card);
			free(card);
		}
	}
	agLLIFree(it);
}

static int pullStartCard() {
	// draw first card until it is a non-wild
	UnoCard *card;
	do {
		if (agLLIsEmpty(deck)) return false;
		card = agLLPop(deck);
		agLLPush(discard, card, sizeof(UnoCard));
		free(card);
	} while (((UnoCard*)agLLPeek(discard))->color == NONE);
	return true;
}

static UnoCard *playBot(UnoPlayer *b) {
	agLLIterator *it = agLLIInit(b->deck);
	UnoCard *card, *toPlay = NULL;
	// orderd highest cards to right - be default try to get rid of them
	while (agLLIHasPrev(it)) {
		card = agLLIPrev(it);
		if (isPlayable(agLLPeek(discard), card)) {
			if (toPlay == NULL) toPlay = card;
			else if (toPlay->color == NONE && card->color != NONE) 
				toPlay = card;
			else if (card->number > toPlay->number) toPlay = card;
		}
	}
	agLLIFree(it);
	if (isBotDelay) usleep(1000 * 1500);

	return toPlay;
}

static int readPlayer(UnoPlayer *p, char **buf, size_t *bufn) {
	agLLIterator *it = agLLIInit(connections);
	struct Packet packet;
	int contains = false;
	struct Connection *c;
	while (agLLIHasNext(it)) {
		c = agLLINext(it);
		if (c->id == p->id) {
			contains = true;
			break;
		}
	}
	agLLIFree(it);

	if (!contains)
		getline(buf, bufn, stdin); 
	else {
		rcvHandle(p, c, &packet);
		agStrntcpy(*buf, packet.data, *bufn);
	}
	return 0;
}

static UnoCard *play(UnoPlayer *p) {
	char *ds, *cs;
	char *cats;
	char spbuf[512];
	size_t bufN = 40;
	char *buf = malloc(bufN);
	if (buf == NULL) {
		perror("Failed to allocate play buffer");
		exit(EXIT_FAILURE);
	}
	char *strErr;
	char *a, *b;
	int ind = -1;
	UnoCard *choice;
	char *rp;
	do {
		cs = cardToString(agLLPeek(discard), LONG_FORMAT);
		ds = deckToString(p->deck, INDEX_FORMAT);

		cats = malloc(strlen(cs) + strlen(ds) + 1 + 128);
		if (cats == NULL) {
			perror("malloc()");
			exit(EXIT_FAILURE);
		}
		cats[0] = '\0';
		snprintf(spbuf, sizeof(spbuf), "> %s <\n", cs);
		strcat(cats, spbuf);
		snprintf(spbuf, sizeof(spbuf), "%s\nEnter the card index to play or 'd' to draw: ", ds);
		strcat(cats, spbuf);

		displayPlayer(p, "%s", cats);

		free(cats);
		free(cs);
		free(ds);
		// getline ret characters read in including newline

		readPlayer(p, &buf, &bufN);

		rp = strchr(buf, '\n');
		if (rp == NULL) continue;
		else *rp = '\0';

		if (strncmp(buf, "d", 3) == 0)
			return NULL;
		errno = 0;
		ind = strtol(buf, &strErr, 10);
		if (*buf == '\0' || *strErr != '\0') {
			displayPlayer(p, "User input was not a number: %c\n", *strErr);
			continue;
		}

		if (errno != 0) {
			perror("User input was invalid");
			ind = -1;
			continue;
		} 
		if (ind < 0 || ind >= p->deck->size) {
			displayPlayer(p, "Index provided (%d) is out of bounds [0-%d)\n", ind, p->deck->size);
			ind = -1;
			continue;
		}
		choice = agLLGet(p->deck, ind);
		if (!isPlayable(agLLPeek(discard), choice)) {
			a = cardToString(choice, LONG_FORMAT);
			b = cardToString(agLLPeek(discard), LONG_FORMAT);
			displayPlayer(p, "You cannot put a %s on a %s\n", a, b);
			free(a);
			free(b);
			continue;
		}
		break;
	} while (1);

	free(buf);
	return choice;
}

static UnoCard *playCard(UnoPlayer *p, UnoCard *c) {
	// remove from deck
	int rgby[4] = {0};
	agLLIterator *it = agLLIInit(p->deck);
	UnoCard *card;
	while (agLLIHasNext(it)) {
		card = agLLINext(it);
		if (card->color < 4)
			rgby[card->color]++;
		if (unoCardEquals(card, c)) {
			agLLIRemove(it); // do not free this card yet, used in loop
			break;
		}
	}
	agLLIFree(it);
	if (c->type == WILD || c->type == WILD_DRAW_4) {
		if (p->isBot) {
			int highest = rgby[0];
			int hi = 0;
			for (int i = 1; i < sizeof(rgby) / sizeof(int); i++) {
				if (rgby[i] > highest) {
					highest = rgby[i];
					hi = i;
				}
			}
			c->color = hi;
		} else {
			size_t len = 10;
			char *in = malloc(len);
			char *rp;
			do {
				displayPlayer(p, "Choose a color <r|b|g|y>: ");
				readPlayer(p, &in, &len);
				rp = strchr(in, '\n'); // set newline to null char
				if (rp == NULL) continue;
				else *rp = '\0';
				if (strlen(in) > 1) {
					displayPlayer(p, "Expected a single character response\n");
					continue;
				}
				if (!strpbrk(in, "rbgyRBGY")) {
					displayPlayer(p, "Invalid char\n");
					continue;
				}
				break;
			} while (1);
			switch(tolower(*in)) {
				case 'r':
					c->color = RED;
					break;
				case 'g':
					c->color = GREEN;
					break;
				case 'b':
					c->color = BLUE;
					break;
				case 'y':
					c->color = YELLOW;
					break;
				default:
					fprintf(stderr, "ERROR: Unexpected char (%c) for wild color selection\n", *in);
					printf("Setting to default color of RED\n");
					c->color = RED;
			}
		}
	}
	char *cStr = cardToString(c, LONG_FORMAT);
	display("%s played a %s\n", p->name, cStr);
	free(cStr);
	agLLPush(discard, c, sizeof(UnoCard));
	return c;
}

static void restoreDeckFromDiscard() {
	UnoCard *card;
	while (!agLLIsEmpty(discard)) {
		card = agLLPop(discard);
		// reset Wild cards
		if (card->type == WILD || card->type == WILD_DRAW_4) 
			card->color = NONE;
		agLLPush(deck, card, sizeof(UnoCard));
		free(card);
	}
}

static UnoCard *drawCard(UnoPlayer *p, int canPlay) {
	// check for an empty deck
	if (agLLIsEmpty(deck)) {
		display("Suffling discard pile into play deck!\n");
		restoreDeckFromDiscard();
	}


	UnoCard *pCard = agLLPop(deck);
	if (canPlay && isPlayable(agLLPeek(discard), pCard)) {
		if (p->isBot)
			return playCard(p, pCard);
		size_t bufn = 32;
		char *buf = malloc(bufn);
		if (buf == NULL) {
			perror("malloc()");
			exit(EXIT_FAILURE);
		}
		char *cstr = cardToString(pCard, LONG_FORMAT);
		char *rp;
		displayPlayer(p, "You just drew a %s, do you play it (y|n): ", cstr);
		free(cstr);
		readPlayer(p, &buf, &bufn);
		rp = strchr(buf, '\n');
		if (rp == NULL) return NULL;
		else *rp = '\0';
		if (strpbrk(buf, "yY") != NULL)
			return playCard(p, pCard);
	} else insertCard(p, pCard);

	return NULL;
}

static int wrapStep(int x, int s, int max) {
	x += s;
	return (x + max) % max;
}

static void showLeaderboard(int round, int rounds) {
	agLinkedList *lb = agLLInit();
	agLLIterator *it, *pit;
	it = agLLIInit(lb);
	pit = agLLIInit(players);
	scorePlayers();
	int isAdded = false;
	int score;
	int cscore;

	UnoPlayer *p;
	while (agLLIHasNext(pit)) {
		p = agLLINext(pit);
		isAdded = false;
		agLLIReset(it);
		cscore = p->score;
		while (agLLIHasNext(it)) {
			score = ((UnoPlayer*)agLLINext(it))->score;
			if (cscore < score) {
				agLLIInsert(it, p, sizeof(UnoPlayer));
				isAdded = true;
				break;
			}
		}
		if (!isAdded) agLLPush(lb, p, sizeof(UnoPlayer));
	}

	const int lbBufN = 256;
	char buf[lbBufN * (players->size + 2)]; // 1 for leaderboard line, 1 for round line
	char ibuf[lbBufN];
	buf[0] = '\0';
	ibuf[0] = '\0';

	strcat(buf, "\n\n");
	snprintf(ibuf, lbBufN, "Round %2d of %-2d\n", round + 1, rounds);
	strcat(buf, ibuf);
	strcat(buf, "\n");
	snprintf(ibuf, lbBufN, "%31s | %-20s", ANSI2(ANSI_FG_YELLOW, "LEADERBOARD"), ANSI2(ANSI_FG_YELLOW, "SCORE"));
	strcat(buf, ibuf);
	strcat(buf, "\n");

	agLLIReset(it);
	UnoPlayer *lbp;
	int place = 0;
	while (agLLIHasNext(it)) {
		lbp = agLLINext(it);
		if (!agLLIHasNext(it))
			snprintf(ibuf, lbBufN, ANSI2(ANSI_FG_RED, "%20s") " | " ANSI2(ANSI_FG_CYAN, "%-10d\n"), lbp->name, lbp->score);
		else if (place < 1)
			snprintf(ibuf, lbBufN, ANSI2(ANSI_FG_YELLOW, "%20s") " | " ANSI2(ANSI_FG_CYAN, "%-10d\n"), lbp->name, lbp->score);
		else if (place < 2)
			snprintf(ibuf, lbBufN, ANSI2(ANSI_FG_GREEN, "%20s") " | " ANSI2(ANSI_FG_CYAN, "%-10d\n"), lbp->name, lbp->score);
		else
			snprintf(ibuf, lbBufN, "%20s | " ANSI2(ANSI_FG_CYAN, "%-10d\n"), lbp->name, lbp->score);

		ibuf[lbBufN-1] = '\0';
		strncat(buf, ibuf, lbBufN);
		place++;
	}
	display("%s\n", buf);
	free(lb);
	agLLIFree(it);
	agLLIFree(pit);
}


static void startGame(int rounds) {
	srand(time(NULL));
	int turn = rand() % players->size;
	int step = 1;
	UnoPlayer *np, *pp;
	char turnbuf[256];
	char tsbuf[128];
	char *arrow = "->";
	deck = createUnoDeck();
	discard = agLLInit();
	shuffleUnoDeck(deck);
	deal();

	display("The Game has started!\n");

	for (int r = 0; r < rounds; r++) {
		while (true) {
			if (agLLIsEmpty(discard) && !pullStartCard()) {
				fprintf(stderr, "Unable to start round, too many cards out of deck.\n");
				break;
			}

			// play
			turnbuf[0] = '\0';
			UnoPlayer *p = agLLGet(players, turn); 
			pp = agLLGet(players, wrapStep(turn, -1, players->size));
			np = agLLGet(players, wrapStep(turn, 1, players->size));

			arrow = step > 0 ? "->" : "<-";

			snprintf(tsbuf, sizeof(tsbuf), "\n%s %s " ANSI2(ANSI_FG_CYAN, "%s") " %s %s\n", pp->name, arrow, p->name, arrow, np->name);
			strcat(turnbuf, tsbuf);
			snprintf(tsbuf, sizeof(tsbuf), "It is " ANSI2(ANSI_FG_CYAN, "%s") "'s turn (%d)\n", p->name, p->deck->size);
			strcat(turnbuf, tsbuf);

			display("%s", turnbuf);
			UnoCard *pCard = p->isBot ? playBot(p) : play(p);
			if (pCard != NULL) {
				playCard(p, pCard);
			} else {
				display("%s needed to draw\n", p->name);
				pCard = drawCard(p, PLAY_ON_DRAW);
			}

			if (agLLIsEmpty(p->deck)) {
				free(pCard);
				break;
			}

			if (pCard == NULL) {
				turn = wrapStep(turn, step, players->size);
				continue;
			}

			if (p->deck->size == 1) {
				display(ANSI2(ANSI_FG_RED, "%s: 'UNO!'\n"), p->name);
				displayPlayer(p, ANSI_CURSOR_UP("1") ANSI(ANSI_ERASE_LN_BK) ANSI_CURSOR_COL_ABS("0") ANSI2(ANSI_FG_GREEN, "%s: 'UNO!'\n" ), p->name);
			}

			switch(pCard->type) {
				case SKIP:
					turn = wrapStep(turn, step, players->size);
					p = agLLGet(players, turn);
					display("%s was skipped\n", p->name);
					break;
				case REVERSE:
					step *= -1;
					display("Reverse, Reverse!");
					break;
				case DRAW_2:
					turn = wrapStep(turn, step, players->size);
					p = agLLGet(players, turn);
					display("%s was forced to draw twice\n", p->name);
					for (int i = 0; i < 2; i++) drawCard(p, NO_PLAY_ON_DRAW);
					break;
				case WILD_DRAW_4:
					turn = wrapStep(turn, step, players->size);
					p = agLLGet(players, turn);
					display("%s was forced to draw four times!\n", p->name);
					for (int i = 0; i < 4; i++) drawCard(p, NO_PLAY_ON_DRAW);
					break;
				case WILD:
				case NUMBER:
					break;
				default:
					fprintf(stderr, "Unrecognized card type: %d\n", pCard->type);
					break;
			}

			turn = wrapStep(turn, step, players->size);
			free(pCard);
		}

		// handle game winning print out 
		showLeaderboard(r, rounds);
		// clear decks
		agLLIterator *it = agLLIInit(players);
		UnoPlayer *p;
		while (agLLIHasNext(it)) {
			p = agLLINext(it);
			while (!agLLIsEmpty(p->deck))
				agLLPush(discard, agLLPop(p->deck), sizeof(UnoCard));
		}
		agLLIFree(it);

		if (r < rounds) {
			// restore decks
			restoreDeckFromDiscard();
			shuffleUnoDeck(deck);
			deal();
		}

		// wait for continue
		size_t ti = 40;
		char *trash = malloc(ti);
		display("Awaiting host to continue...\n");
		printf("Press <ENTER> to continue\n");
		getline(&trash, &ti, stdin);
		if (r < rounds)
			display(ANSI(ANSI_ERASE_DISP_ALL) "Round " ANSI2(ANSI_FG_CYAN, "%d!"), r + 1);
	}

	struct Packet packet;
	packet.cmd = EXIT;
	if (!agLLIsEmpty(connections))
		multiHandle(&packet);

	agLLIterator *it = agLLIInit(players);
	while (agLLIHasNext(it))
		agLLFree(((UnoPlayer*)agLLINext(it))->deck);
	agLLIFree(it);
	agLLFree(deck);
	agLLFree(discard);
}

void sigintHandler(int sigNum) {
	printf("\nTerminating Application\n");
	struct Packet p;
	p.cmd = EXIT;
	if (!agLLIsEmpty(connections))
		multiHandle(&p);
	closeOnline(connections);
	exit(EXIT_SUCCESS);
}

int main(int argc, char *argv[]) {
	const char *optStr = "b:hj:i:mnr:";
	extern char *optarg;
	extern int optind, optopt;
	players = agLLInit();
	connections = agLLInit();
	int optret;
	int botN = 3;
	int isBotDefault = true;
	int isHost = false;
	int isClient = false;
	int port = 0;
	int rounds = 3;
	char ip[32];
	int playerN;
	int gameN;
	struct Packet packet;

	while ((optret = getopt(argc, argv, optStr)) != -1) {
		switch(optret) {
			case 'b':
				isBotDefault = false;
				botN = atoi(optarg);
				printf("botN=%d\n", botN);
				break;
			case 'h':
				printf("Options: uno [-b: bots=int bots] [-n: no bot-delay] [-m: multiplayer host]\n"
						"[-j: join multiplayer=int port] [-i: join multiplayer=string ip] [-r: rounds-set=int rounds]\n"
						"<name> [other players ...]\n\n"
						"* Note that the order of the options do not matter.\n"
						"* Player names can be a maximum of %d characters.\n"
						"* There can be no more than %d players in a game.\n"
						"* Bots default to 3, subtracting one bot for each additional player.\n"
						"* There can be a max of 10 players in the game between bots and players.\n"
						"* To host an online game, use the -m for multiplayer flag. Supply friends with the listed\n"
						"port that is printed to output when setup to join, which they can enter by using\n"
						"the -j option followed by the port AND the -i option followed by the ip address of the host.\n"
						"* To specify the number of rounds use the -r option. Default is 3.\n"
						, UNO_PLAYER_NAMEN, PLAYERN);
				return EXIT_SUCCESS;
			case 'r':
				rounds = atoi(optarg);
				if (rounds < 1) {
					printf("Need to play at least one round (entered %d)\n", rounds);
				} else if (rounds > 20) {
					printf("There is no need to play more than 20 rounds, %d rounds? Come on.\n", rounds);
				}
			case 'n':
				isBotDelay = false;
			case 'm':
				isHost = true;
				break;
			case 'j':
				isClient = true;
				port = atoi(optarg);
				if (port < 1) {
					printf("Port needs to be > 0. Ports should always be above 1024.");
					return EXIT_FAILURE;
				}
				break;
			case 'i':
				agStrntcpy(ip, optarg, sizeof(ip));
				break;
			default:
				fprintf(stderr, "Unexpected option: %c\n", optopt);
				return EXIT_FAILURE;
		}
	}

	// interupt handler
	signal(SIGINT, sigintHandler);

	playerN =  argc - optind;

	if (playerN <= 0) {
		printf("Expect: uno [-b: bots] <name> [other players ...]\n"
				"At least one player is required\n");
		return EXIT_FAILURE;
	}

	if (isHost || isClient) {
		// multiplayer setup
		if (isClient && isHost) {
			fprintf(stderr, "Cannot host and join at the same time!\n");
			return EXIT_FAILURE;
		}

		if (isClient) {
			joinOnline(connections, ip, port);
			playOnline(argv[optind]);
			return EXIT_SUCCESS;
		}

		// report the port that we are hosting on
		// increment playerN for each person who joined
		if ((hostOnline(PLAYERN - playerN, connections, NULL)) < 0) {
			printf("There was an error setting hosting.\n");
			return EXIT_FAILURE;
		}
		playerN += connections->size;
		UnoPlayer *p;
		agLLIterator *it = agLLIInit(connections);
		struct Connection *c;
		while (agLLIHasNext(it)) {
			c = agLLINext(it);
			p = initUnoPlayer("0", false);
			// to start all players should init with their name
			rcvHandle(p, c, &packet);
			agStrntcpy(p->name, packet.data, sizeof(p->name));
			agLLPush(players, p, sizeof(UnoPlayer));
			free(p);
		}
		agLLIFree(it);
	}

	if (isBotDefault) {
		botN -= playerN - 1;
		if (botN < 0) botN = 0;
	}

	gameN = playerN + botN;

	if (gameN > 10) {
		fprintf(stderr, "Max of 10 players (%d) and bots (%d) total (%d)\n", playerN, botN, gameN);
		return EXIT_FAILURE;
	}

	UnoPlayer *p;
	for (int i = connections->size, boti = 0; i < gameN; i++) {
		if (optind < argc)
			agLLPush(players, (p = initUnoPlayer(argv[optind++], false)), sizeof(UnoPlayer));
		else
			agLLPush(players, (p = initUnoPlayer(botNames[boti++], true)), sizeof(UnoPlayer));
		free(p);
	}

	if (gameN <= 1) {
		puts("Can't play a game solo unfortunately! Add some bots if you need to with the -b <# bots> option.");
		return 1;
	}

	startGame(rounds);

	free(players);
	free(connections);

	return 0;
}
