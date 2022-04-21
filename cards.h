#ifndef CARDS_H 
#include "agLinkedList.h"

#define CARDS_H
#define UNO_DECKN 108
#define UNO_PLAYER_NAMEN 20
#define UNO_COLORN (UNO_DECKN / 4)
#define UNO_CARD_STRN 64

#define INDEX_FORMAT 'i'
#define SHORT_FORMAT 's'
#define LONG_FORMAT 'l'

typedef enum UnoCardType {
	NUMBER, SKIP, REVERSE, DRAW_2, WILD, WILD_DRAW_4
} UnoCardType;

typedef enum UnoCardColor {
	RED, BLUE, GREEN, YELLOW, NONE
} UnoCardColor;

typedef struct UnoCard {
	UnoCardType type;
	UnoCardColor color;
	unsigned char number;
} UnoCard;


agLinkedList *createUnoDeck();
void shuffleUnoDeck(agLinkedList *deck);
char *deckToString(agLinkedList *deck, const char format);
char *cardToString(UnoCard *card, const char format);
int isPlayable(const UnoCard *current, const UnoCard *next);
int unoCardEquals(const UnoCard *a, const UnoCard *b);

#endif
