/* HW2 CONTROLLER
 * - Read the root README.md first. 
 * - Complete only the TODOs, including choosing your button numbers.
 * - Do not modify other files or provided code.
 * - Keep the supplied function names, parameters and global variable names.
 * - You may add your own local or global variables in hw.c and choose their names.
 * - Declare globals outside functions. Keep all supplied names unchanged.
 * - Agree on command strings with the mainboard. Do not use '|'.
 * - End every message you send with '\n'.
 */
#include "hw.h"
#include <stdio.h>
#include <string.h>

/* TODO Task 5: choose your controller buttons
 * - Replace each 0 below with your chosen button number from 1 to 8.
 * - Choose three different buttons for betting.
 * - Choose three different buttons for playing.
 * - You may reuse a button in different stages.
 * - The supplied screen displays the button numbers you choose.
 *
 * Hint:
 * - 0 means not chosen yet. For example, one button can increase the bet
 *   during BETTING and request Draw during your own turn.
 */
const uint8_t bet_increase_button = 0;
const uint8_t bet_decrease_button = 0;
const uint8_t bet_confirm_button = 0;
const uint8_t draw_button = 0;
const uint8_t double_button = 0;
const uint8_t stop_button = 0;

void ControllerCommandReceived(const char *received_command)
{
    /* TODO Task 1: assignment and acknowledgement
     * - Recognize your assignment command and read the player number (1 or 2).
     * - Store it in player_number and call SendAcknowledgement().
     * - Set redraw_flag = 1 to refresh the screen.
     * - Reply again if main repeats the assignment.
     *
     * TODO Task 4: status updates
     * - Read the four values: stage, balance, bet and finished.
     * - Check that sscanf() read all four values.
     * - Compare the received stage with the old game_stage before replacing it.
     * - When entering a new BETTING stage, set selected_bet_amount to
     *   INITIAL_BET (10). If the received balance is less than 10, use 0.
     * - Keep the selection when another update arrives in the same BETTING stage.
     * - Store game_stage, player_balance, player_bet and hand_finished.
     * - After a valid update, set waiting_for_main_reply = 0 and redraw_flag = 1.
     *
     * Hints:
     * - Example commands are START,1 and STATUS,2,190,10,0.
     * - sscanf() reads numbers from your agreed command format.
     * - Stage values are WAIT_FOR_PLAYERS through RESULTS (see the README).
     * - You receive only the text before '|'.
     * - Cards, bet_confirmed and results are already read by the supplied code.
     * - A player with less than 10 balance sits out the next round.
     */
    (void)received_command;
}

void SendAcknowledgement(void)
{
    /* TODO Task 1: acknowledgement
     * - Build your acknowledgement string containing player_number.
     * - End it with '\n' and send directly through huart1.
     *
     * Hints:
     * - Create char message_to_send[MESSAGE_SIZE] and fill it with snprintf().
     */
}

void ChangeSelectedBet(int8_t direction)
{
    /* TODO Task 2: change the selected bet
     * - If direction is 1, increase selected_bet_amount by BET_STEP (10).
     * - If direction is -1, decrease selected_bet_amount by BET_STEP (10).
     * - Keep selected_bet_amount between 0 and player_balance.
     * - Change only the selection; do not deduct player_balance yourself.
     * - Set redraw_flag = 1 after changing the selection.
     *
     * Hints:
     * - In HandlePlayerButton(), call ChangeSelectedBet(1) for increase
     *   and ChangeSelectedBet(-1) for decrease.
     * - redraw_flag = 1 asks the supplied code to refresh the screen.
     */
    (void)direction;
}

uint8_t SendBet(void)
{
    /* TODO Task 2: send the bet
     * - Send only when selected_bet_amount is greater than zero.
     * - Build your bet command containing selected_bet_amount and a final '\n'.
     * - Send directly through huart1 using HAL_UART_Transmit().
     * - Return 1 after calling HAL_UART_Transmit().
     * - If the bet is zero, return 0 without sending anything.
     *
     * Hints:
     * - snprintf() can build a command such as BET,10\n.
     * - Return 1 means you made a request. HandlePlayerButton() then sets
     *   waiting_for_main_reply = 1 to block requests until main replies.
     * - Main's status reply supplies the confirmed bet and actual balance.
     */
    return 0;
}

uint8_t SendPlayerAction(uint8_t action)
{
    /* TODO Task 3: player actions
     * - Convert ACTION_DRAW, ACTION_DOUBLE and ACTION_STOP into your agreed
     *   command strings, each ending with '\n'.
     * - Send directly through huart1 using HAL_UART_Transmit().
     * - Return 1 after calling HAL_UART_Transmit().
     * - For an unknown action, return 0 without sending anything.
     *
     * Hints:
     * - strcpy() can copy a command string into a character array.
     * - Use the same Draw, Double and Stop strings as the mainboard.
     * - Do not deal cards here; the mainboard's supplied game functions do it.
     */
    (void)action;
    return 0;
}

void HandlePlayerButton(uint8_t pressed_button_number)
{
    /* TODO Task 5: controller button actions and permissions
     * - Use your six button constants to check pressed_button_number.
     * - Before assignment, or while waiting_for_main_reply is 1, do nothing.
     * - During BETTING, allow the betting buttons only while bet_confirmed is 0:
     *   increase -> ChangeSelectedBet(1), decrease -> ChangeSelectedBet(-1),
     *   confirm -> SendBet().
     * - During your own turn, allow action buttons only while hand_finished is 0:
     *   call SendPlayerAction(ACTION_DRAW), SendPlayerAction(ACTION_DOUBLE)
     *   or SendPlayerAction(ACTION_STOP) for the matching button.
     * - If SendBet() or SendPlayerAction() returns 1,
     *   set waiting_for_main_reply = 1.
     * - During PLAYER1_TURN, PLAYER2_TURN, DEALER_TURN or RESULTS, if a chosen
     *   Draw/Double/Stop button is pressed outside your own turn or after your
     *   hand finishes, set show_action_warning = 1 and redraw_flag = 1.
     * - Return without sending a command for that blocked action.
     * - Ignore unused buttons. During WAIT_FOR_PLAYERS or confirmed BETTING,
     *   do nothing; betting buttons may reuse the same numbers as action buttons.
     *
     * Hints:
     * - Player 1's turn: player_number == 1 and game_stage == PLAYER1_TURN.
     * - Player 2's turn: player_number == 2 and game_stage == PLAYER2_TURN.
     * - This function receives a button number from 1 to 8 for each new press.
     * - Handle betting before checking playing-turn permission.
     * - waiting_for_main_reply blocks requests until a valid status reply.
     * - show_action_warning = 1 requests red NOT ALLOWED text. The supplied
     *   display clears this flag after drawing; the text stays until the next
     *   screen refresh. No timer, HAL_Delay() or extra drawing function is needed.
     * - Follow game_stage from main; do not add a separate controller FSM.
     */
    (void)pressed_button_number;
}
