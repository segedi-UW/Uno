#include "cards.h"
#include <stdlib.h>
#include <stdio.h>
#include <string.h>
#include <limits.h>
#include <time.h>
#include "agLLIterator.h"
#include "agAnsi.h"

// Matches the UnoCardType and UnoCardColor enums in cards.h
const static char *colors[] = {"Red", "Blue", "Green", "Yellow", ""};
const static char *names[] = {
	 "Zero", "One", "Two", "Three", "Four", "Five", "Six",
	"Seven", "Eight", "Nine", "Skip", "Reverse", "Draw2",
	"Wild", "Wild Draw4"
};
const static char *abbrs[] = {
	"0", "1", "2", "3", "4", "5", "6",
	"7", "8", "9", "S", "R", "D2", "W", "W4"
};
const static char *cabbrs[] = {"R", "B", "G", "Y", ""};
static char cardBuffer[UNO_CARD_STRN * 2];

agLinkedList *createUnoDeck() {
	UnoCard deck[UNO_DECKN];
	if (deck == NULL) {
		perror("Failed to allocate deck");
		exit(EXIT_FAILURE);
	}

	UnoCard *card = deck;
	int cn;
	for (int c = 0; c < 4; c++) {
		// skip the zero cards for first iteration: i = 1
		for (int i = 1; i < 26; i++, card++) {
			cn = i % 13;
			card->type = cn < 10 ? 0 : cn - 9;
			card->color = c;
			card->number = cn < 10 ? cn : 9 + card->type;
		}
		// add one wild and one wild4
		for (int i = 0; i < 2; i++, card++) {
			card->color = NONE;
			card->type = i == 0 ? WILD : WILD_DRAW_4;
			card->number = 10 + card->type;
		}
	}

	agLinkedList *list = agLLInit();
	if (list == NULL) {
		perror("Failed to allocate LinkedList");
		exit(EXIT_FAILURE);
	}

	card = deck;
	UnoCard *c;
	for (int i = 0; i < UNO_DECKN; i++, card++) {
		c = malloc(sizeof(UnoCard));
		*c = *card;
		agLLPush(list, c, sizeof(UnoCard));
	}

	return list;
}

void shuffleUnoDeck(agLinkedList *deck) {
	agLLIterator *it = agLLIInit(deck);
	agLinkedList *list = agLLInit();
	while (agLLIHasNext(it)) {
		agLLAppend(list, agLLINext(it), sizeof(UnoCard));
	}
	agLLClear(deck);

	srand(time(NULL) % INT_MAX);
	UnoCard *g;
	for (int ind; !agLLIsEmpty(list);) {
		ind = rand() % list->size;
		g = agLLGet(list, ind);
		agLLAppend(deck, g, sizeof(UnoCard));
		agLLRemoveFree(list, ind);
	}

	agLLFree(list);
}

int unoCardEquals(const UnoCard *a, const UnoCard *b) {
	return a->type == b->type && a->color == b->color
		&& a->number == b->number;
}

int isPlayable(const UnoCard *c, const UnoCard *n) {
	return (n->type == WILD || n->type == WILD_DRAW_4)
		|| c->color == n->color || c->number == n->number;
}

static const char *getName(UnoCard *card, const char format) {
	const char **p = (format == INDEX_FORMAT || format == SHORT_FORMAT) ? abbrs : names;
	return card->type == NUMBER ? p[card->number] : p[9 + card->type];
}

static const char *getColor(UnoCard *card, const char format) {
	const char **p = (format == INDEX_FORMAT || format == SHORT_FORMAT) ? cabbrs : colors;
	switch (card->color) {
		case RED:
			snprintf(cardBuffer, sizeof(cardBuffer), ANSI(ANSI_FG_RED) "%s", p[card->color]);
			break;
		case BLUE:
			snprintf(cardBuffer, sizeof(cardBuffer), ANSI(ANSI_FG_BLUE) "%s", p[card->color]);
			break;
		case GREEN:
			snprintf(cardBuffer, sizeof(cardBuffer), ANSI(ANSI_FG_GREEN) "%s", p[card->color]);
			break;
		case YELLOW:
			snprintf(cardBuffer, sizeof(cardBuffer), ANSI(ANSI_FG_YELLOW) "%s", p[card->color]);
			break;
		case NONE:
			snprintf(cardBuffer, sizeof(cardBuffer), ANSI(ANSI_FG_MAGENTA) "%s", p[card->color]);
			break;
		default:
			return p[card->color];
	}
	return cardBuffer;
}

char *cardToString(UnoCard *card, const char format) {
	char pbuf[UNO_CARD_STRN];
	char *sep = format == LONG_FORMAT ? " " : "";
	pbuf[0] = '\0';
	sprintf(pbuf,ANSI(ANSI_BG_BLACK) "%s%s%s" ANSI(ANSI_RESET), getColor(card, format), sep, getName(card, format));
	pbuf[UNO_CARD_STRN - 1] = '\0';
	return strdup(pbuf);
}

char *deckToString(agLinkedList *deck, const char format) {
	char buf[2048];
	char ibuf[8];
	char *ls;

	buf[0] = '\0';

	UnoCard *card;
	char *cardStr;
	agLLIterator *it = agLLIInit(deck);
	for (int i = 0; agLLIHasNext(it); i++) {
		card = agLLINext(it);
		ls = agLLIHasNext(it) ? ", " : "";
		cardStr = cardToString(card, format);
		
		if (i != 0 && i % 8 == 0)
			strcat(buf, "\n");
		if (format == INDEX_FORMAT) {
			snprintf(ibuf, sizeof(ibuf), "%d:", i);
			ibuf[sizeof(ibuf)-1] = '\0';
			strcat(buf, ibuf);
		}
		strncat(buf, cardStr, UNO_CARD_STRN);
		strcat(buf, ls);
		free(cardStr);
	}
	agLLIFree(it);
	return strdup(buf);
}

