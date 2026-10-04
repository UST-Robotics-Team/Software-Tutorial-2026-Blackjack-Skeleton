# Homework 2: Three-board Blackjack

## Your goal

Connect one mainboard and two controllers to play blackjack. The mainboard is the dealer. It stores the deck, deals cards, checks the rules and calculates balances. Each controller lets one player choose a bet and request Draw, Double or Stop.

The game and display code are provided. Your work is to finish the **six TODO tasks** that connect your UART commands and buttons to the game.

**Only complete the TODOs in these files:**

- [Controller/Core/Src/hw.c](Controller/Core/Src/hw.c)
- [MainBoard/Core/Src/hw.c](MainBoard/Core/Src/hw.c)

Choosing the six controller button numbers is also a TODO in `Controller/Core/Src/hw.c`. Keep the supplied function names, parameters and global variable names. You may add your own local or global variables in `hw.c` and choose their names. Declare globals outside functions. Keep all supplied names unchanged.

**Do not modify any other files, libraries, `main.c`, headers or `.ioc` settings. Only implement what the TODO comments ask for.** UART receiving, interrupts, message buffering, card drawing and game rules are already provided.

The supplied code calls your command handlers, button handlers, `SendStartCommand()` and `SendPlayerUpdate()` automatically. Your controller handlers must call `SendAcknowledgement()`, `ChangeSelectedBet()`, `SendBet()` and `SendPlayerAction()` when appropriate.

## Board setup and HC-05 wiring

Open the `MainBoard` folder for the mainboard project, or the `Controller` folder for a player controller project. **Do not open the homework root folder as the project folder.** Each board has its own project.

Use the STM32 build and flashing procedure taught in the tutorials; the projects are already configured. And remember to re-generate the code using CubeMx 

Use **four HC-05 modules**: connect **one to each controller** and **two to the mainboard**. One mainboard HC05 communicates with Controller 1; the other communicates with Controller 2.

**Remember to configure and pair the modules before use.** Each mainboard module must connect to its corresponding controller. Set the mainboard modules as masters and controller modules as slaves. Use **115200 baud, 8 data bits, no parity and 1 stop bit**, matching the provided UART configuration. [HC-05 module reference](https://shillehtek.com/blogs/shillehtek-product-manuals/hc-05-6pin-bluetooth-module-no-button-manual).

## Game flow

```text
Connect both players → Bet → Player 1 → Player 2 → Dealer → Results
                         ↑                                  |
                         +----------------------------------+
```

1. Main sends each player an assignment until it receives an acknowledgement. Both players must connect.
2. Press mainboard SW1 to enter betting. Players select and confirm their bets.
3. Press SW1 after both bets are confirmed. Main deals the cards and starts Player 1's turn.
4. The active player chooses Draw, Double or Stop. Main sends the updated information back.
5. After a player finishes, SW1 advances the stage. Both players must finish before the dealer's turn.
6. During the dealer's turn, SW2 short press selects Draw or Stop; holding it for at least one second confirms the choice.
7. Main calculates the results. SW1 at the results screen begins the next betting round.

Balances start at 200. The selected bet starts at 10 and changes in steps of 10. A bet cannot exceed the available balance. Draw adds a card; reaching 21 or going over 21 finishes the hand. Double doubles the bet, adds one card and finishes. Stop finishes the hand. The provided code also handles initial blackjack, payouts and players without enough balance to bet.

## Choose your controller buttons

At the top of controller `hw.c`, replace each `0` with your chosen button number from **1 to 8**:

| Constant | Button function |
|---|---|
| `bet_increase_button` | Increase the selected bet |
| `bet_decrease_button` | Decrease the selected bet |
| `bet_confirm_button` | Confirm the selected bet |
| `draw_button` | Request Draw |
| `double_button` | Request Double |
| `stop_button` | Request Stop |

Use three different buttons for betting and three different buttons for playing. You may reuse a button in different stages. For example, the same button can increase the bet during betting and request Draw during your turn. The supplied display shows the button numbers you choose.

Mainboard SW1 changes the stage and SW2 controls the dealer's choice. Their roles are already assigned.

## Agree on your command strings

Choose the command names and format together. Both mainboard and controller code must understand the same strings. For example:

```text
START,1\n
ACK,1\n
BET,100\n
DRAW\n
DOUBLE\n
STOP\n
```

You may choose different names. End each complete command with `\n`. The provided receiver collects the characters and removes this ending before calling your command handler.

An acknowledgement is a reply confirming the assignment was received. For example, main sends `START,1` and the controller replies `ACK,1`. Main then stops repeating that assignment.

Send directly with `HAL_UART_Transmit()`, for example:

```c
char message_to_send[32] = "HELLO\n";
HAL_UART_Transmit(&huart1, (uint8_t *)message_to_send,
                  strlen(message_to_send), 100);
```

### Status messages and the `|` separator

Main sends four values to each controller: **stage, balance, bet and finished**. For example, your status text could be:

```text
STATUS,2,190,10,0
```

This means Player 1's turn, balance 190, bet 10 and the hand is still active.

The supplied code adds card information after your text:

```text
STATUS,2,190,10,0|card information added by the supplied code\n
```

**You only write and read the text before `|`.** Do not use `|` in your own commands.

On main, first build your status text without `\n`, then call `AddCardDataToMessage()`. It adds `|`, the card information and the final `\n` for you. Send the completed message.

On the controller, the supplied receiver reads the card information automatically. Your `ControllerCommandReceived()` receives only your status text, such as `STATUS,2,190,10,0`.

| Stage number | Named constant |
|---|---|
| 0 | `WAIT_FOR_PLAYERS` |
| 1 | `BETTING` |
| 2 | `PLAYER1_TURN` |
| 3 | `PLAYER2_TURN` |
| 4 | `DEALER_TURN` |
| 5 | `RESULTS` |

Use the named constants in your code. Finished is `0` while the hand can still act and `1` when it has ended.

## Recommended implementation order

Use this order to check your progress as you work. The task numbers and marking stay the same.

**Task 1 → Task 4 → Task 2 + betting part of Task 5 → Task 3 + remaining Task 5 → Task 6**

1. **Task 1 — Connect both players.** Implement assignment and acknowledgement on both boards. Check that the controllers show Player 1 and Player 2, and that mainboard SW1 only allows betting after both connect.
2. **Task 4 — Receive the current status.** Implement status sending on mainboard and status handling on the controller. Press SW1 to enter betting. Check that both controllers show betting, balance 200 and a selected bet of 10.
3. **Task 2 + betting part of Task 5 — Choose and confirm bets.** Choose your betting button numbers and handle increase, decrease and confirmation. Check the selected amounts, the accepted bets on mainboard and the updated controller balances.
4. **Task 3 + remaining Task 5 — Play the player turns.** Choose your action buttons and implement Draw, Double, Stop and turn permissions. After both bets are confirmed, press SW1 to deal. Check the action results and the red NOT ALLOWED warning for blocked actions. SW1 advances after each player finishes.
5. **Task 6 — Control the dealer.** Implement SW2 short/long press handling. Check selection, confirmation once, and release without an extra action. Finish the round and use SW1 to begin the next betting round.

## Task 1: Assignment and acknowledgement (2 marks)

Tasks work together. The notes in parentheses below identify other tasks needed to demonstrate each marking criterion. They are not an order for completing the tasks.

**Mainboard:** `SendStartCommand()` and the acknowledgement part of `MainCommandReceived()`.

- Main sends Player 1's assignment through `huart1` and Player 2's through `huart2`.
- Main checks that the reply's player number matches the UART's player, then sets `player_connected[player_number] = 1`.

**Controller:** `SendAcknowledgement()` and the assignment part of `ControllerCommandReceived()`.

- Controller reads and stores `player_number`, replies with an acknowledgement and sets `redraw_flag = 1`.
- Reply again if main repeats the assignment.

**Hint:** `snprintf()` puts numbers into a string. `sscanf()` reads numbers from a string. The main handler's `player_number` parameter tells you which controller sent the message.

### Marking

- Each controller displays its correct player number: **1 mark**. (This can be tested after finishing Task 1; no other task is needed.)
- At startup, SW1 cannot enter betting with only one controller connected. Betting becomes available after both connect: **1 mark**. (This can be tested on the mainboard after finishing Task 1; Task 4 is also needed to show betting on the controllers.)

## Task 2: Betting (2 marks)

**Mainboard:** the betting part of `MainCommandReceived()`.

- Main reads the requested amount and calls `SetPlayerBet(player_number, amount)`. This supplied function checks the bet and deducts it if accepted.

**Controller:** `ChangeSelectedBet()` and `SendBet()`.

- When the increase button is pressed, call `ChangeSelectedBet(1)`. Increase `selected_bet_amount` by `BET_STEP`, which is 10.
- When the decrease button is pressed, call `ChangeSelectedBet(-1)`. Decrease it by 10.
- Keep the selection between 0 and `player_balance`. Change only the selection; do not deduct the balance yourself.
- Set `redraw_flag = 1` after changing the selection. This tells the provided code to refresh the screen.
- `SendBet()` sends the selected amount when it is greater than zero. Return `1` after calling `HAL_UART_Transmit()`; return `0` if the bet is zero and you send nothing.

**Hint:** A return value of `1` here tells the button handler that you made a request. It then sets `waiting_for_main_reply = 1` to block more requests until main replies. The reply supplies the actual bet and balance.

### Marking

- The selected bet starts at 10, increases/decreases by 10 and stays between zero and the displayed balance: **1 mark**. (To achieve this, also finish Tasks 1, 4 and 5.)
- After confirmation, main displays the correct bet and the controller displays the correctly reduced balance: **1 mark**. (To achieve this, also finish Tasks 1, 4 and 5.)

## Task 3: Player actions (2 marks)

**Mainboard:** the action part of `MainCommandReceived()`.

- Main recognizes each command and calls `DrawCardForPlayer()`, `DoublePlayerBet()` or `FinishPlayerTurn()`.

**Controller:** `SendPlayerAction()`.

- Convert `ACTION_DRAW`, `ACTION_DOUBLE` and `ACTION_STOP` into your agreed command strings and send them through `huart1`.
- Return `1` after calling `HAL_UART_Transmit()`. Return `0` for an unknown action without sending anything.

**Hint:** `strcmp(received_command, "DRAW") == 0` checks whether a received string is `DRAW`. Main's game functions check whether the action is allowed. The supplied main code schedules the reply; you do not send another update from this handler.

### Marking

- Allowed actions produce the correct screen changes: Draw adds one card; Double adds one card, doubles the bet and finishes the hand; Stop finishes without adding a card: **2 marks**. (To achieve this, also finish Tasks 1, 2, 4 and 5.)

## Task 4: Status updates (2 marks)

**Mainboard:** `SendPlayerUpdate()`.

- Main builds a string containing `game_stage`, `players[player_number].balance`, `.bet` and `.finished` in your agreed order.
- Call `AddCardDataToMessage(message_to_send, MESSAGE_SIZE, player_number)` to add the card data and newline. If it returns `0`, return `0` without sending.
- Send through `huart1` for Player 1 or `huart2` for Player 2. Return `1` after calling `HAL_UART_Transmit()`.

**Controller:** the status part of `ControllerCommandReceived()`.

- Controller reads all four values and stores `game_stage`, `player_balance`, `player_bet` and `hand_finished`.
- When a new `BETTING` stage begins, reset `selected_bet_amount` to `INITIAL_BET` (10). If the player has less than 10, use zero; that player sits out. Reset the selected bet only when a new betting round begins, as you may receive update again
- After a valid update, set `waiting_for_main_reply = 0` and `redraw_flag = 1`.

**Hint:** Check that `sscanf()` read four values. Compare the received stage with the old `game_stage` before replacing it. Card data, bet confirmation and results have already been handled by the supplied receiver.

### Marking

- After main changes the stage, both controllers display the appropriate betting, turn, waiting, dealer or result information: **1 mark**. (Finish Task 1 to test the betting display; also finish Tasks 2, 3 and 5 to demonstrate all later stages.)
- After a reply, the controller displays the updated cards, balance, bet and hand status, and the next allowed button action works. A new betting round displays a selected bet of 10 when the player has enough balance: **1 mark**. (To demonstrate a complete round and the next round, also finish Tasks 1, 2, 3 and 5.)

For these status checks, the supplied SW1 handler can advance from the dealer's turn to results. Task 6 is needed to demonstrate the dealer's SW2 short/long press controls.

## Task 5: Controller button actions and permissions (2 marks)

**Controller:** `HandlePlayerButton()` and the six button constants.

- Choose your button numbers and use those constants when checking the pressed button.
- Before assignment, or while `waiting_for_main_reply = 1`, do nothing.
- During `BETTING`, allow increase, decrease and confirm only while `bet_confirmed` is zero.
- During your own turn, allow Draw, Double and Stop only while `hand_finished` is zero.
- If `SendBet()` or `SendPlayerAction()` returns `1`, set `waiting_for_main_reply = 1`.
- During `PLAYER1_TURN`, `PLAYER2_TURN`, `DEALER_TURN` or `RESULTS`, if a chosen Draw/Double/Stop button is pressed outside your own turn or after your hand finishes, set `show_action_warning = 1` and `redraw_flag = 1`.
- Return without sending a command for that blocked action. The controller must block it itself.
- Ignore unused buttons. During `WAIT_FOR_PLAYERS` or confirmed `BETTING`, do nothing; betting buttons may reuse the same numbers as action buttons.

**Hint:** Player 1 can act when `player_number == 1` and `game_stage == PLAYER1_TURN`. Use the corresponding check for Player 2. Your function receives one button number, 1..8, for each new press. Handle betting before checking playing-turn permission.

**Warning hint:** `show_action_warning = 1` asks the supplied display code to show **NOT ALLOWED** in red. Also set `redraw_flag = 1` to refresh the screen. The supplied code clears the warning flag after drawing; the text stays on screen until the next refresh. You do not need a timer, `HAL_Delay()` or an extra drawing function. Before assignment or while waiting for a reply, keep ignoring buttons as described above.

### Marking

- The chosen button numbers shown on the TFT match the buttons that perform each function: **1 mark**. (To demonstrate all betting and playing buttons, also finish Tasks 1, 2, 3 and 4.)
- Pressing a chosen action button during another player's turn, the dealer's turn, results or after the hand finishes shows **NOT ALLOWED** in red, without changing cards or bets: **1 mark**. (Finish Tasks 1, 2 and 4 to test the wrong-turn warning; also finish Task 3 to demonstrate the finished-hand, dealer and result cases.)

## Task 6: Dealer short and long press (4 marks)

**Mainboard:** `HandleDealerButton(button_is_pressed, current_time)`.

- The provided input is `1` when SW2 is pressed and `0` when released. The time is in milliseconds.
- Only act during `DEALER_TURN`.
- A short press released before 1000 ms switches `selected_dealer_option` between `0` (Draw) and `1` (Stop). Set `redraw_flag = 1`.
- A hold of at least 1000 ms confirms the selected option once. Call `DrawCardForDealer()` or `FinishDealerTurn()`.
- Holding longer must not repeat the action. Releasing after a long press must not also switch the option.

**Hint:** Store the previous button level, the press start time and whether the long action already happened. Calculate the held time with `current_time - button_press_start_time`. Outside the dealer's turn, do not perform any game action.

### Marking

- Short press changes the displayed Draw/Stop selection without performing the action: **1 mark**. (To reach the dealer's turn, also finish Tasks 1 to 5.)
- Long press performs the selected action once; continuing to hold causes no additional action: **1 mark**. (To reach the dealer's turn, also finish Tasks 1 to 5.)
- After confirming Draw with a long press, releasing SW2 adds no extra card and does not change the Draw/Stop selection: **1 mark**. (To reach the dealer's turn, also finish Tasks 1 to 5.)
- SW2 causes no game changes outside the dealer's turn: **1 mark**. (This can be tested at startup without other tasks; also finish Tasks 1 to 5 to test the betting and player stages.)

## Marking summary

Marks are awarded for the behaviour demonstrated on the TFT screens.

| Task | What the TFT demonstrates | Marks |
|---|---|---:|
| 1: Connection | Correct player numbers; betting requires both players | 2 |
| 2: Betting | Change the selected bet and confirm it; balances update | 2 |
| 3: Actions | Draw, Double and Stop produce the correct results | 2 |
| 4: Status updates | Correct stages, balances, bets and updated hands | 2 |
| 5: Controller buttons | Chosen buttons work when allowed; blocked actions show red NOT ALLOWED text | 2 |
| 6: Dealer button | Short press selects; long press confirms once | 4 |
| **Total** | | **14** |

## Variables you will use

| Name | Meaning | Who updates it |
|---|---|---|
| `player_number` | The controller's assigned player: 1 or 2 | Controller Task 1 stores the number received in the assignment. On mainboard, it is a function parameter identifying which player the function handles. |
| `player_connected[number]` | Main received that player's acknowledgement | Mainboard Task 1 sets it to `1` after checking the acknowledgement. Supplied startup code initializes it to `0`. |
| `players[1]`, `players[2]` | Main's stored player information | Supplied mainboard game functions update it. Call those functions for bets/actions and read these values in Task 4. |
| `game_stage` | The current stage | Supplied mainboard FSM changes it. Controller Task 4 stores the stage received from main. |
| `INITIAL_BALANCE` | Starting balance: 200 | Provided constant; use its value without changing it. |
| `INITIAL_BET` | Starting bet selection: 10 | Provided constant; use its value without changing it. |
| `BET_STEP` | Increase/decrease amount: 10 | Provided constant; use its value without changing it. |
| `selected_bet_amount` | The bet currently selected on the controller; starts at 10 | Controller Task 2 changes it locally. Task 4 resets it when a new betting round begins. Supplied startup code initializes it. |
| `player_balance`, `player_bet` | Balance and confirmed bet received from main | Controller Task 4 stores the received values. Supplied mainboard game functions decide the actual balance and bet; do not deduct the controller's balance yourself. |
| `bet_confirmed` | `1` after main accepts the bet, or marks the player as sitting out | The supplied controller receiver reads it from main's card/flag data. Your TODOs only read it. |
| `hand_finished` | `1` when this player's hand has ended | Supplied mainboard game functions decide it. Controller Task 4 stores the finished value from the status reply. |
| `waiting_for_main_reply` | `1` blocks requests until main replies | Controller Task 5 sets it to `1` after a request. Task 4 resets it to `0` after a valid status reply. |
| `redraw_flag` | Set to `1` to ask the supplied code to refresh the screen | Your TODOs and supplied game code set it to `1` when needed. The supplied display loop resets it to `0` after drawing. |
| `show_action_warning` | Set to `1` with `redraw_flag` to show red NOT ALLOWED text | Controller Task 5 sets it to `1` for a blocked action. The supplied display code resets it to `0` after drawing. |
| `selected_dealer_option` | `0` means Draw; `1` means Stop | Mainboard Task 6 switches it on a short press. Supplied mainboard code resets it to `0` at startup and after stage changes. |

The supplied game functions return `1` if they accept an action and `0` if they reject it. Their rules, card dealing and stage changes are already implemented.
