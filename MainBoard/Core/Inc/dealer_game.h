#ifndef DEALER_GAME_H
#define DEALER_GAME_H
#include "cards.h"

/* One source of game data: dealer = 0, players = 1 and 2. */
extern GameStage game_stage;
extern Player players[3];
extern BlackJackDeck deck;
extern uint8_t player_connected[3];
extern uint8_t redraw_flag;
extern uint8_t selected_dealer_option;

void DealerGame_Init(void);
void DealerGame_Run(void);

/* Rules and the single main-board FSM. 1 = accepted, 0 = rejected. */
void InitializeGame(void);
void BeginNextRound(void);
uint8_t NextGameStage(uint32_t shuffle_seed);
uint8_t SetPlayerBet(uint8_t player_number, int32_t amount);
uint8_t DrawCardForPlayer(uint8_t player_number);
uint8_t DoublePlayerBet(uint8_t player_number);
uint8_t FinishPlayerTurn(uint8_t player_number);
uint8_t DrawCardForDealer(void);
uint8_t FinishDealerTurn(void);

/* Provided: appends reserved card/flag data and a newline to your status text. */
uint8_t AddCardDataToMessage(char *message_to_send, uint16_t capacity, uint8_t player_number);

#endif
