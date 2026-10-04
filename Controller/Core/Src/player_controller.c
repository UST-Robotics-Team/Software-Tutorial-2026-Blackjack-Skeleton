#include "player_controller.h"
#include "hw.h"
#include "cards.h"
#include "buttons.h"
#include "uart_commands.h"
#include "lcd/lcd.h"
#include "usart.h"
#include <stdio.h>
#include <stdlib.h>

/* A controller follows the main board's stage; no second FSM or display snapshot. */
GameStage game_stage;
uint8_t player_number;
int32_t player_balance, player_bet, selected_bet_amount;
uint8_t bet_confirmed, hand_finished, waiting_for_main_reply, redraw_flag;
uint8_t show_action_warning;
static Hand player_hand;
static GameResult player_result;

static CommandBuffer received_command;
static uint8_t received_byte;
static volatile uint8_t receive_started;
static Button buttons[8];

static void StartReceiving(void)
{
    if (!receive_started && HAL_UART_Receive_IT(&huart1, &received_byte, 1) == HAL_OK)
        receive_started = 1;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *uart)
{
    if (uart != &huart1) return;
    receive_started = 0;
    CollectCommandByte(&received_command, received_byte);
    StartReceiving();
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *uart)
{
    if (uart != &huart1) return;
    received_command.character_count = 0;
    received_command.discard_line = 1;
    receive_started = 0;
    StartReceiving();
}

/* Read one comma-separated number; card packing stays inside the application. */
static uint8_t ReadNumber(const char **position, long *number)
{
    char *end;
    *number = strtol(*position, &end, 10);
    if (end == *position) return 0;
    if (*end == ',') *position = end + 1;
    else if (!*end) *position = end;
    else return 0;
    return 1;
}

/* Only reserved card metadata is parsed here. The student's status remains theirs.
 * Main's AddCardDataToMessage appends |confirmed,result,count,rank,suit,...
 * Before delivering the prefix, commit cards/flags, but leave game_stage alone.
 */
static uint8_t ReceiveCardData(char *message)
{
    char *separator = strchr(message, '|');
    if (!separator) return 1;  /* Ordinary assignment or another student command. */
    const char *position = separator + 1;
    long confirmed, result, count;
    if (!ReadNumber(&position, &confirmed) || !ReadNumber(&position, &result) ||
        !ReadNumber(&position, &count) || confirmed < 0 || confirmed > 1 ||
        result < 0 || result > RESULT_BLACKJACK || count < 0 || count > MAX_HAND_CARDS)
        return 0;
    Hand incoming = {0};
    incoming.count = count;
    for (uint8_t i = 0; i < incoming.count; ++i) {
        long rank, suit;
        if (!ReadNumber(&position, &rank) || !ReadNumber(&position, &suit) ||
            rank < 1 || rank > 13 || suit < 0 || suit > 3) return 0;
        incoming.cards[i].rank = rank;
        incoming.cards[i].suit = suit;
    }
    if (*position) return 0;
    player_hand = incoming;
    player_result = result;
    bet_confirmed = confirmed;
    *separator = '\0';
    return 1;
}



static void ProcessCommands(void)
{
    if (!received_command.message_ready) return;
    char command[COMMAND_SIZE];
    strcpy(command, received_command.message);
    received_command.message_ready = 0;
    if (ReceiveCardData(command)) ControllerCommandReceived(command);
}







static uint8_t ReadPlayerButton(uint8_t index)
{
    static GPIO_TypeDef *const ports[8] = {
        Button1_GPIO_Port, Button2_GPIO_Port, Button3_GPIO_Port, Button4_GPIO_Port,
        Button5_GPIO_Port, Button6_GPIO_Port, Button7_GPIO_Port, Button8_GPIO_Port
    };
    static const uint16_t pins[8] = {
        Button1_Pin, Button2_Pin, Button3_Pin, Button4_Pin,
        Button5_Pin, Button6_Pin, Button7_Pin, Button8_Pin
    };
    return HAL_GPIO_ReadPin(ports[index], pins[index]) == GPIO_PIN_RESET;
}

static void HandleButtons(void)
{
    /* Debounce/edges are supplied. Students decide the game action. */
    for (uint8_t i = 0; i < 8; ++i)
        if (ButtonUpdate(&buttons[i], ReadPlayerButton(i), HAL_GetTick()) == BUTTON_PRESS)
            HandlePlayerButton(i + 1);
}

static void DrawPlayerScreen(void)
{
    static const char *const stages[] = {
        "", "BETTING", "PLAYER 1 TURN", "PLAYER 2 TURN", "DEALER TURN", "RESULT"
    };
    static const char *const results[] = {
        "NO BET", "YOU WIN", "YOU LOSE", "PUSH", "BUST", "BLACKJACK!"
    };
    char text[32];
    tft_force_clear();
    snprintf(text, sizeof text, "PLAYER %u", player_number);
    CenterText(0, text);
    if (game_stage == BETTING) {
        if (waiting_for_main_reply) {
            CenterText(1, "SENDING BET");
            CenterText(5, "WAIT FOR REPLY");
        } else if (bet_confirmed) {
            CenterText(1, "BET CONFIRMED");
            CenterText(5, "WAIT FOR DEAL");
        } else {
            CenterText(1, "HOW MUCH TO BET?");
            snprintf(text, sizeof text, "B%u +10 B%u -10", bet_increase_button, bet_decrease_button);
            CenterText(5, text);
            snprintf(text, sizeof text, "B%u CONFIRM", bet_confirm_button);
            CenterText(6, text);
        }
        int32_t displayed_bet = bet_confirmed ? player_bet : selected_bet_amount;
        snprintf(text, sizeof text, "BET: %ld", (long)displayed_bet);
        CenterText(3, text);
    } else if (game_stage != WAIT_FOR_PLAYERS) {
        CenterText(1, game_stage == RESULTS ? results[player_result] : stages[game_stage]);
        uint8_t score = GetHandScore(&player_hand);
        const char *status;
        if (game_stage == RESULTS) status = "ROUND FINISHED";
        else if (!player_bet) status = "NO BET";
        else if (IsBlackjack(&player_hand)) status = "BLACKJACK!";
        else if (score > 21) status = "BUST";
        else if (score == 21) status = "21 - HAND DONE";
        else if (hand_finished) status = "HAND STOPPED";
        else if (game_stage != (player_number == 1 ? PLAYER1_TURN : PLAYER2_TURN)) status = "WAIT YOUR TURN";
        else if (player_hand.count == 2 && player_balance >= player_bet) {
            snprintf(text, sizeof text, "%uDRAW %uDBL %uSTOP", draw_button, double_button, stop_button);
            status = text;
        } else {
            snprintf(text, sizeof text, "%u:DRAW %u:STOP", draw_button, stop_button);
            status = text;
        }
        CenterText(2, status);
        if (player_hand.count) {
            snprintf(text, sizeof text, "SUM:%u BET:%ld", score, (long)player_bet);
            CenterText(8, text);
        }
    }
    snprintf(text, sizeof text, "BALANCE:%ld", (long)player_balance);
    CenterText(char_max_y - 1, text);
    if (show_action_warning) {
        /* Row 2 is above the cards. Replace the normal action prompt. */
        tft_print_colored(0, 2, "NOT ALLOWED     ", RED, BLACK);
        show_action_warning = 0;
    }
    (void)tft_update(0);
    DrawCards(&player_hand);
}

void PlayerController_Init(void)
{
    memset(&received_command, 0, sizeof received_command);
    memset(&player_hand, 0, sizeof player_hand);
    player_bet = bet_confirmed = hand_finished = player_result = 0;
    player_balance = INITIAL_BALANCE;
    game_stage = WAIT_FOR_PLAYERS;
    player_number = receive_started = waiting_for_main_reply = redraw_flag = 0;
    show_action_warning = 0;
    selected_bet_amount = INITIAL_BET;
    for (uint8_t i = 0; i < 8; ++i)
        ButtonInit(&buttons[i], ReadPlayerButton(i), HAL_GetTick());
    tft_init(PIN_ON_TOP, BLACK, WHITE, RED, GREY);
    StartReceiving();
}

void PlayerController_Run(void)
{
    StartReceiving();
    ProcessCommands();
    HandleButtons();
    if (redraw_flag && player_number) { DrawPlayerScreen(); redraw_flag = 0; }
    HAL_Delay(5);
}
