#ifndef CARDS_H
#define CARDS_H

#include <stdint.h>

#define BLACK_JACK_CARD_COUNT 5U
#define BLACK_JACK_DECK_SIZE 52U
#define TFT_CARD_WIDTH 22
#define TFT_CARD_HEIGHT 44

typedef enum {
    BLACK_JACK_HEARTS,
    BLACK_JACK_DIAMONDS,
    BLACK_JACK_CLUBS,
    BLACK_JACK_SPADES
} BlackJackSuit;

typedef struct {
    uint8_t rank; /* 1 = Ace, 2..10, 11 = Jack, 12 = Queen, 13 = King */
    BlackJackSuit suit;
} BlackJackCard;

typedef struct {
    BlackJackCard cards[BLACK_JACK_DECK_SIZE];
    uint8_t next_card; /* Cards before this index have already been dealt. */
} BlackJackDeck;

/* Create all 52 unique cards, shuffle them, and reset next_card to zero.
   Same seed gives the same order for testing. Use a varying seed (for example
   HAL_GetTick() when Start is pressed) for different rounds. Null is ignored.
   This initializes the deck only; it does not initialize or draw the TFT. */
void BlackJack_Init(BlackJackDeck *deck, uint32_t seed);
/* Deal once: returns 1 on success, 0 for null arguments or an empty deck. */
uint8_t BlackJack_DeckDraw(BlackJackDeck *deck, BlackJackCard *card);
uint8_t BlackJack_DeckRemaining(const BlackJackDeck *deck);

/* Draw a 22x44-pixel card at its top-left pixel position after TFT init.
   number: 1=Ace, 2..10, 11=Jack, 12=Queen, 13=King.
   Invalid ranks/suits or positions where the whole card cannot fit are ignored.
   Draws immediately without clearing the screen; no tft_update() needed. */
void tft_draw_card(int16_t x, int16_t y, uint8_t number, BlackJackSuit suit);

#define INITIAL_BALANCE 200
#define INITIAL_BET 10
#define BET_STEP 10
#define MAX_HAND_CARDS 12

typedef enum {
    WAIT_FOR_PLAYERS, BETTING, PLAYER1_TURN, PLAYER2_TURN, DEALER_TURN, RESULTS
} GameStage;
typedef enum {
    RESULT_NONE, RESULT_WIN, RESULT_LOSE, RESULT_PUSH, RESULT_BUST, RESULT_BLACKJACK
} GameResult;

typedef struct {
    BlackJackCard cards[MAX_HAND_CARDS];
    uint8_t count;
} Hand;

/* The main stores three Players: 0 = dealer, 1/2 = players.
   A controller stores only its own Player. No separate display snapshot. */
typedef struct {
    Hand hand;
    int32_t balance;
    int32_t bet;
    uint8_t bet_confirmed;
    uint8_t finished;
    GameResult result;
} Player;

uint8_t GetHandScore(const Hand *hand);
uint8_t IsBlackjack(const Hand *hand);
void DrawCards(const Hand *hand);
void CenterText(uint8_t row, const char *text);

#endif
