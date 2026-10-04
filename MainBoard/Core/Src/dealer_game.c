#include "dealer_game.h"
#include "hw.h"
#include "buttons.h"
#include "uart_commands.h"
#include "lcd/lcd.h"
#include "usart.h"
#include <stdio.h>
#include <stdlib.h>

GameStage game_stage;
Player players[3];                    /* 0 = dealer, 1/2 = players */
BlackJackDeck deck;
uint8_t player_connected[3];

static CommandBuffer received_commands[3];
static uint8_t received_byte[3];
static volatile uint8_t receive_started[3];
static uint8_t send_update_flag[3];
uint8_t redraw_flag;
uint8_t selected_dealer_option; /* 0 = Draw, 1 = Stop */
static Button stage_button, dealer_button;
static uint32_t last_start_command_time;

static UART_HandleTypeDef *PlayerUART(uint8_t player_number)
{
    return player_number == 1 ? &huart1 : &huart2;
}

static void GameChanged(void)
{
    redraw_flag = 1;
    send_update_flag[1] = send_update_flag[2] = 1;
}

void InitializeGame(void)
{
    memset(players, 0, sizeof players);
    memset(player_connected, 0, sizeof player_connected);
    memset(&deck, 0, sizeof deck);
    for (uint8_t number = 0; number <= 2; ++number)
        players[number].balance = INITIAL_BALANCE;
    game_stage = WAIT_FOR_PLAYERS;
    selected_dealer_option = 0;
    GameChanged();
}

void BeginNextRound(void)
{
    for (uint8_t number = 0; number <= 2; ++number) {
        players[number].hand.count = 0;
        players[number].bet = 0;
        players[number].result = RESULT_NONE;
        /* A player without enough credit sits out this round. */
        players[number].bet_confirmed = number && players[number].balance < BET_STEP;
        players[number].finished = players[number].bet_confirmed;
    }
    game_stage = BETTING;
    GameChanged();
}

static uint8_t DealCard(Hand *hand)
{
    if (hand->count >= MAX_HAND_CARDS) return 0;
    if (!BlackJack_DeckDraw(&deck, &hand->cards[hand->count])) return 0;
    ++hand->count;
    return 1;
}

static uint8_t IsPlayerTurn(uint8_t number)
{
    if (number != 1 && number != 2) return 0;
    return game_stage == (number == 1 ? PLAYER1_TURN : PLAYER2_TURN) &&
           !players[number].finished;
}

uint8_t SetPlayerBet(uint8_t player_number, int32_t amount)
{
    if (player_number != 1 && player_number != 2) return 0;
    Player *player = &players[player_number];
    if (game_stage != BETTING || player->bet_confirmed || amount <= 0 ||
        amount > player->balance || amount % BET_STEP) return 0;
    player->bet = amount;
    player->balance -= amount;
    player->bet_confirmed = 1;
    GameChanged();
    return 1;
}

uint8_t DrawCardForPlayer(uint8_t player_number)
{
    if (!IsPlayerTurn(player_number)) return 0;
    if (!DealCard(&players[player_number].hand)) return 0;
    if (GetHandScore(&players[player_number].hand) >= 21)
        players[player_number].finished = 1;
    GameChanged();
    return 1;
}

uint8_t DoublePlayerBet(uint8_t player_number)
{
    if (!IsPlayerTurn(player_number)) return 0;
    Player *player = &players[player_number];
    if (player->hand.count != 2 || player->balance < player->bet) return 0;
    if (!DealCard(&player->hand)) return 0;
    player->balance -= player->bet;
    player->bet *= 2;
    player->finished = 1;
    GameChanged();
    return 1;
}

uint8_t FinishPlayerTurn(uint8_t player_number)
{
    if (!IsPlayerTurn(player_number)) return 0;
    players[player_number].finished = 1;
    GameChanged();
    return 1;
}

static void PayWinnings(void)
{
    uint8_t dealer_score = GetHandScore(&players[0].hand);
    uint8_t dealer_blackjack = IsBlackjack(&players[0].hand);
    for (uint8_t number = 1; number <= 2; ++number) {
        Player *player = &players[number];
        uint8_t player_score = GetHandScore(&player->hand);
        int32_t payout = 0;
        if (!player->bet) { player->result = RESULT_NONE; continue; }
        if (player_score > 21) player->result = RESULT_BUST;
        else if (IsBlackjack(&player->hand) && !dealer_blackjack) {
            player->result = RESULT_BLACKJACK;
            payout = player->bet + player->bet * 3 / 2;
        } else if (dealer_blackjack && !IsBlackjack(&player->hand))
            player->result = RESULT_LOSE;
        else if (player_score == dealer_score) {
            player->result = RESULT_PUSH;
            payout = player->bet;
        } else if (dealer_score > 21 || player_score > dealer_score) {
            player->result = RESULT_WIN;
            payout = player->bet * 2;
        } else player->result = RESULT_LOSE;
        player->balance += payout;
        players[0].balance += player->bet - payout;
    }
}

uint8_t NextGameStage(uint32_t shuffle_seed)
{
    if (!player_connected[1] || !player_connected[2]) return 0;
    switch (game_stage) {
    case WAIT_FOR_PLAYERS:
    case RESULTS:
        BeginNextRound();
        break;
    case BETTING:
        if (!players[1].bet_confirmed || !players[2].bet_confirmed) return 0;
        BlackJack_Init(&deck, shuffle_seed);
        for (uint8_t round = 0; round < 2; ++round) {
            for (uint8_t number = 1; number <= 2; ++number)
                if (players[number].bet) DealCard(&players[number].hand);
            DealCard(&players[0].hand);
        }
        for (uint8_t number = 1; number <= 2; ++number)
            if (IsBlackjack(&players[number].hand)) players[number].finished = 1;
        game_stage = PLAYER1_TURN;
        break;
    case PLAYER1_TURN:
        if (!players[1].finished) return 0;
        game_stage = PLAYER2_TURN;
        break;
    case PLAYER2_TURN:
        if (!players[1].finished || !players[2].finished) return 0;
        game_stage = DEALER_TURN;
        break;
    case DEALER_TURN:
        PayWinnings();
        game_stage = RESULTS;
        break;
    default:
        return 0;
    }
    selected_dealer_option = 0;
    GameChanged();
    return 1;
}

uint8_t DrawCardForDealer(void)
{
    if (game_stage != DEALER_TURN || GetHandScore(&players[0].hand) >= 21) return 0;
    if (!DealCard(&players[0].hand)) return 0;
    GameChanged();
    return 1;
}

uint8_t FinishDealerTurn(void)
{
    if (game_stage != DEALER_TURN) return 0;
    return NextGameStage(0);
}

/* ---- UART: newline-terminated commands, no separate protocol module. ---- */
static void StartReceiving(uint8_t player_number)
{
    if (!receive_started[player_number] &&
        HAL_UART_Receive_IT(PlayerUART(player_number), &received_byte[player_number], 1) == HAL_OK)
        receive_started[player_number] = 1;
}

void HAL_UART_RxCpltCallback(UART_HandleTypeDef *uart)
{
    uint8_t number = uart == &huart1 ? 1 : uart == &huart2 ? 2 : 0;
    if (!number) return;
    receive_started[number] = 0;
    CollectCommandByte(&received_commands[number], received_byte[number]);
    StartReceiving(number);
}

void HAL_UART_ErrorCallback(UART_HandleTypeDef *uart)
{
    uint8_t number = uart == &huart1 ? 1 : uart == &huart2 ? 2 : 0;
    if (!number) return;
    received_commands[number].character_count = 0;
    received_commands[number].discard_line = 1;
    receive_started[number] = 0;
    StartReceiving(number);
}



static void ProcessCommands(void)
{
    for (uint8_t number = 1; number <= 2; ++number) {
        if (!received_commands[number].message_ready) continue;
        char command[COMMAND_SIZE];
        strcpy(command, received_commands[number].message);
        received_commands[number].message_ready = 0;
        MainCommandReceived(number, command);
        /* Every request gets an authoritative reply, even if rejected. */
        if (player_connected[number]) send_update_flag[number] = 1;
    }
}

uint8_t AddCardDataToMessage(char *message_to_send, uint16_t capacity, uint8_t number)
{
    if ((number != 1 && number != 2) || capacity == 0) return 0;
    const Player *player = &players[number];
    size_t used = strlen(message_to_send);
    if (used >= capacity || strchr(message_to_send, '\n')) return 0;
    int added = snprintf(message_to_send + used, capacity - used, "|%u,%u,%u",
        player->bet_confirmed, (unsigned)player->result, player->hand.count);
    if (added < 0 || (size_t)added >= capacity - used) return 0;
    used += added;
    for (uint8_t i = 0; i < player->hand.count; ++i) {
        added = snprintf(message_to_send + used, capacity - used, ",%u,%u",
            player->hand.cards[i].rank, (unsigned)player->hand.cards[i].suit);
        if (added < 0 || (size_t)added >= capacity - used) return 0;
        used += added;
    }
    if (used + 2 > capacity) return 0;
    message_to_send[used++] = '\n';
    message_to_send[used] = '\0';
    return 1;
}

static void SendCommands(void)
{
    uint32_t current_time = HAL_GetTick();
    uint8_t repeat_start = current_time - last_start_command_time >= 1000;
    if (repeat_start) last_start_command_time = current_time;
    for (uint8_t number = 1; number <= 2; ++number) {
        if (!player_connected[number]) {
            if (repeat_start) SendStartCommand(number);
        } else if (send_update_flag[number]) {
            if (SendPlayerUpdate(number)) send_update_flag[number] = 0;
        }
    }
}

/* ---- Main-board buttons and screen read the game data directly. ---- */
static void HandleMainButtons(void)
{
    uint32_t current_time = HAL_GetTick();
    uint8_t stage_event = ButtonUpdate(&stage_button,
        HAL_GPIO_ReadPin(BTN1_GPIO_Port, BTN1_Pin) == GPIO_PIN_RESET, current_time);
    ButtonUpdate(&dealer_button,
        HAL_GPIO_ReadPin(BTN2_GPIO_Port, BTN2_Pin) == GPIO_PIN_RESET, current_time);
    if (stage_event == BUTTON_PRESS) NextGameStage(current_time);
    /* Called every loop; the TODO handler checks whether it is dealer turn. */
    HandleDealerButton(dealer_button.stable_pressed, current_time);
}

static void DrawDealerScreen(void)
{
    static const char *const stages[] = {
        "", "BETTING", "PLAYER 1 TURN", "PLAYER 2 TURN", "DEALER TURN", "RESULT"
    };
    static const char *const results[] = {"NO BET", "WIN", "LOSE", "PUSH", "BUST", "BLACKJACK"};
    char text[32];
    tft_force_clear();
    CenterText(0, "DEALER");
    if (game_stage != WAIT_FOR_PLAYERS) {
        CenterText(1, stages[game_stage]);
        if (game_stage == PLAYER1_TURN || game_stage == PLAYER2_TURN) {
            uint8_t number = game_stage == PLAYER1_TURN ? 1 : 2;
            if (players[number].finished) {
                snprintf(text, sizeof text, "PLAYER %u FINISHED", number);
                CenterText(1, text);
            }
            CenterText(2, players[number].finished ? "SW1 TO CONTINUE" : "WAIT FOR PLAYER");
        } else if (game_stage == DEALER_TURN) {
            snprintf(text, sizeof text, "DEALER SUM:%u", GetHandScore(&players[0].hand));
            CenterText(1, text);
            CenterText(2, selected_dealer_option ? "> STOP" : "> DRAW");
            CenterText(8, "SW1 TO CONTINUE");
        } else {
            CenterText(2, "SW1 TO CONTINUE");
        }
        if (game_stage == BETTING || game_stage == RESULTS) {
            for (uint8_t number = 1; number <= 2; ++number) {
                if (game_stage == BETTING)
                    snprintf(text, sizeof text, "P%u BET:%ld %s", number, (long)players[number].bet,
                        players[number].bet_confirmed ? "OK" : "?");
                else snprintf(text, sizeof text, "P%u %s", number, results[players[number].result]);
                CenterText(3 + number, text);
            }
        }
        if (players[0].hand.count && game_stage != DEALER_TURN) {
            snprintf(text, sizeof text, "SUM:%u", GetHandScore(&players[0].hand));
            CenterText(8, text);
        }
    }
    snprintf(text, sizeof text, "BALANCE:%ld", (long)players[0].balance);
    CenterText(char_max_y - 1, text);
    (void)tft_update(0);
    if (game_stage != RESULTS) DrawCards(&players[0].hand);
}

void DealerGame_Init(void)
{
    InitializeGame();
    memset(received_commands, 0, sizeof received_commands);
    for (uint8_t number = 0; number <= 2; ++number) receive_started[number] = 0;
    ButtonInit(&stage_button, HAL_GPIO_ReadPin(BTN1_GPIO_Port, BTN1_Pin) == GPIO_PIN_RESET, HAL_GetTick());
    ButtonInit(&dealer_button, HAL_GPIO_ReadPin(BTN2_GPIO_Port, BTN2_Pin) == GPIO_PIN_RESET, HAL_GetTick());
    tft_init(PIN_ON_TOP, BLACK, WHITE, RED, GREY);
    StartReceiving(1);
    StartReceiving(2);
    last_start_command_time = HAL_GetTick() - 1000U;
    SendCommands();
}

void DealerGame_Run(void)
{
    StartReceiving(1);
    StartReceiving(2);
    ProcessCommands();
    HandleMainButtons();
    SendCommands();
    if (redraw_flag) { DrawDealerScreen(); redraw_flag = 0; }
    HAL_Delay(5);
}
