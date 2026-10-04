#ifndef PLAYER_CONTROLLER_H
#define PLAYER_CONTROLLER_H
#include "cards.h"
/* Supplied scalar values. You do not need to understand the card structure. */
extern GameStage game_stage;
extern uint8_t player_number;
extern int32_t player_balance, player_bet, selected_bet_amount;
extern uint8_t bet_confirmed, hand_finished, waiting_for_main_reply, redraw_flag;
/* Set to 1 with redraw_flag to request the red NOT ALLOWED text. */
extern uint8_t show_action_warning;
void PlayerController_Init(void);
void PlayerController_Run(void);
#endif
