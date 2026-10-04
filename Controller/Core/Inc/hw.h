#ifndef HW_H
#define HW_H
#include "player_controller.h"
#include "usart.h"
#define MESSAGE_SIZE 256
/* Action names are independent of your UART command strings. */
enum { ACTION_DRAW, ACTION_DOUBLE, ACTION_STOP };
extern const uint8_t bet_increase_button, bet_decrease_button, bet_confirm_button;
extern const uint8_t draw_button, double_button, stop_button;
void ControllerCommandReceived(const char *received_command);
void SendAcknowledgement(void);
void ChangeSelectedBet(int8_t direction);
uint8_t SendBet(void);
uint8_t SendPlayerAction(uint8_t action);
void HandlePlayerButton(uint8_t pressed_button_number);
#endif
