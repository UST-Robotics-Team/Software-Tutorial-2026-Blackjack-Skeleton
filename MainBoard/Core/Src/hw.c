/* HW2 MAINBOARD
 * - Read the root README.md first.
 * - Complete only the TODOs. Do not modify other files or provided code.
 * - Keep the supplied function names, parameters and global variable names.
 * - You may add your own local or global variables in hw.c and choose their names.
 * - Declare globals outside functions. Keep all supplied names unchanged.
 * - Agree on command strings with the controller. Do not use '|'.
 * - End commands with '\n'. Task 4's helper adds the status newline for you.
 */
#include "hw.h"
#include <stdio.h>
#include <string.h>

void SendStartCommand(uint8_t player_number)
{
    /* TODO Task 1: assignment and acknowledgement
     * - Build your assignment command containing player_number.
     * - End the command with '\n'.
     * - Send through huart1 for Player 1 or huart2 for Player 2.
     *
     * Hints:
     * - snprintf() puts a number into a string, for example START,1\n.
     * - The supplied timer repeats the assignment until acknowledgement.
     * - Do not add a retry loop or HAL_Delay().
     */
    (void)player_number;
}

void MainCommandReceived(uint8_t player_number, const char *received_command)
{
    /* TODO Task 1: acknowledgement
     * - Recognize your acknowledgement command and read its player number.
     * - Check that the reply's number matches the player_number parameter.
     * - If they match, set player_connected[player_number] = 1.
     *
     * TODO Task 2: betting
     * - Recognize your bet command and read the requested amount.
     * - Call SetPlayerBet(player_number, amount).
     * - The supplied function checks the bet and deducts it if accepted.
     *
     * TODO Task 3: player actions
     * - For your Draw command, call DrawCardForPlayer(player_number).
     * - For your Double command, call DoublePlayerBet(player_number).
     * - For your Stop command, call FinishPlayerTurn(player_number).
     *
     * Hints:
     * - player_number tells you which controller sent the command.
     * - sscanf() reads numbers from a string.
     * - strcmp(received_command, "DRAW") == 0 checks for the string DRAW.
     * - The supplied game functions check whether each action is allowed.
     * - They return 1 if accepted and 0 if rejected.
     * - The supplied main code schedules the reply. Do not send an update here.
     */
    (void)player_number;
    (void)received_command;
}

uint8_t SendPlayerUpdate(uint8_t player_number)
{
    /* TODO Task 4: status updates
     * - Build your status string using these four values in the agreed order:
     *   game_stage, players[player_number].balance, .bet and .finished.
     * - Do not add '|' or '\n' to this status text.
     * - Call AddCardDataToMessage(message_to_send, MESSAGE_SIZE, player_number).
     * - If it returns 0, return 0 without sending anything.
     * - Send through huart1 for Player 1 or huart2 for Player 2.
     * - Return 1 after calling HAL_UART_Transmit().
     *
     * Hints:
     * - Create char message_to_send[MESSAGE_SIZE] and fill it with snprintf().
     * - An example status text is STATUS,2,190,10,0.
     * - The helper adds '|', card information and the final '\n'.
     * - The controller must read the four values in the same order.
     */
    (void)player_number;
    return 0;
}

void HandleDealerButton(uint8_t button_is_pressed, uint32_t current_time)
{
    /* TODO Task 6: dealer short and long press
     * - Only act during DEALER_TURN.
     * - On a short press released before 1000 ms, switch selected_dealer_option
     *   between 0 (Draw) and 1 (Stop), then set redraw_flag = 1.
     * - When held for at least 1000 ms, confirm the selected option once:
     *   call DrawCardForDealer() for Draw or FinishDealerTurn() for Stop.
     * - Continuing to hold must not repeat the action.
     * - Releasing after a long press must not draw again or switch the option.
     *
     * Hints:
     * - This function is called every loop. The input is already debounced.
     * - button_is_pressed is 1 when pressed and 0 when released.
     * - Outside DEALER_TURN, do not perform any game action.
     */
    (void)button_is_pressed;
    (void)current_time;
}
