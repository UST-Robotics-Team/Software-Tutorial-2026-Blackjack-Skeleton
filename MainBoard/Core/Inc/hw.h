#ifndef HW_H
#define HW_H
#include "dealer_game.h"
#include "usart.h"
#define MESSAGE_SIZE 256
/* Main calls these functions automatically. Complete their bodies in hw.c. */
void SendStartCommand(uint8_t player_number);
void MainCommandReceived(uint8_t player_number, const char *received_command);
uint8_t SendPlayerUpdate(uint8_t player_number);
void HandleDealerButton(uint8_t button_is_pressed, uint32_t current_time);
#endif
