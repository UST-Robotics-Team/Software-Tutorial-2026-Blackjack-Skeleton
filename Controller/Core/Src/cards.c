#include "cards.h"
#include <stdlib.h>
#include <string.h>
#include "lcd/lcd.h"
#include "lcd/lcd_graphics.h"

/* Three-bit rows for 0..9, A, J, Q, K and an invalid-rank marker. */
static const uint8_t rank_font[][5] = {
    {7, 5, 5, 5, 7}, {2, 6, 2, 2, 7}, {7, 1, 7, 4, 7},
    {7, 1, 7, 1, 7}, {5, 5, 7, 1, 1}, {7, 4, 7, 1, 7},
    {7, 4, 7, 5, 7}, {7, 1, 2, 2, 2}, {7, 5, 7, 5, 7},
    {7, 5, 7, 1, 7}, {2, 5, 7, 5, 5}, {1, 1, 1, 5, 7},
    {7, 5, 5, 7, 1}, {5, 5, 6, 5, 5}, {7, 1, 2, 0, 2}
};

static void DrawGlyph(int16_t x, int16_t y, uint8_t glyph,
                      int16_t scale, uint16_t color)
{
    for (int16_t row = 0; row < 5; ++row) {
        for (int16_t col = 0; col < 3; ++col) {
            if (rank_font[glyph][row] & (1U << (2 - col))) {
                fillRect(x + col * scale, y + row * scale,
                         scale, scale, color);
            }
        }
    }
}

static void DrawRank(int16_t x, int16_t y, uint8_t rank,
                     int16_t scale, uint16_t color)
{
    if (rank == 10U) {
        DrawGlyph(x, y, 1, scale, color);
        DrawGlyph(x + 4 * scale, y, 0, scale, color);
    } else {
        uint8_t glyph = 14;
        if (rank == 1U) glyph = 10;
        else if (rank >= 2U && rank <= 9U) glyph = rank;
        else if (rank >= 11U && rank <= 13U) glyph = rank;
        DrawGlyph(x, y, glyph, scale, color);
    }
}

static void DrawSuit(int16_t x, int16_t y, BlackJackSuit suit,
                     int16_t s, uint16_t color)
{
    switch (suit) {
    case BLACK_JACK_HEARTS:
        fillCircle(x - 2*s, y - s, 2*s, color);
        fillCircle(x + 2*s, y - s, 2*s, color);
        fillTriangle(x - 4*s, y, x + 4*s, y, x, y + 5*s, color);
        break;
    case BLACK_JACK_DIAMONDS:
        fillTriangle(x, y - 5*s, x - 4*s, y, x + 4*s, y, color);
        fillTriangle(x - 4*s, y, x + 4*s, y, x, y + 5*s, color);
        break;
    case BLACK_JACK_CLUBS:
        fillCircle(x, y - 3*s, 2*s, color);
        fillCircle(x - 2*s, y, 2*s, color);
        fillCircle(x + 2*s, y, 2*s, color);
        fillTriangle(x, y, x - 2*s, y + 5*s, x + 2*s, y + 5*s, color);
        break;
    case BLACK_JACK_SPADES:
        fillTriangle(x, y - 5*s, x - 4*s, y, x + 4*s, y, color);
        fillCircle(x - 2*s, y, 2*s, color);
        fillCircle(x + 2*s, y, 2*s, color);
        fillTriangle(x, y, x - 2*s, y + 5*s, x + 2*s, y + 5*s, color);
        break;
    default:
        break;
    }
}

static void DrawCard(int16_t x, int16_t y, int16_t card_width,
                     int16_t card_height, uint8_t number, BlackJackSuit suit)
{
    int16_t scale = (card_width >= 36) ? 2 : 1;
    uint16_t color = (suit == BLACK_JACK_HEARTS ||
                      suit == BLACK_JACK_DIAMONDS) ? RED : BLACK;
    fillRect(x, y, card_width, card_height, WHITE);
    drawRect(x, y, card_width, card_height, BLACK);
    DrawRank(x + 3, y + 3, number, scale, color);
    DrawSuit(x + card_width / 2, y + card_height / 2,
             suit, scale, color);
    int16_t rank_width = ((number == 10U) ? 7 : 3) * scale;
    DrawRank(x + card_width - rank_width - 3,
             y + card_height - 5 * scale - 3, number, scale, color);
}

void tft_draw_card(int16_t x, int16_t y, uint8_t number, BlackJackSuit suit)
{
    int16_t width = (tft_orientation % 2) ? MAX_HEIGHT : MAX_WIDTH;
    int16_t height = (tft_orientation % 2) ? MAX_WIDTH : MAX_HEIGHT;
    if (number < 1 || number > 13 ||
        (unsigned)suit > BLACK_JACK_SPADES ||
        x < 0 || y < 0 || x > width - TFT_CARD_WIDTH ||
        y > height - TFT_CARD_HEIGHT) return;
    DrawCard(x, y, TFT_CARD_WIDTH, TFT_CARD_HEIGHT, number, suit);
}







void BlackJack_Init(BlackJackDeck *deck, uint32_t seed)
{
    if (deck == 0) return;
    uint8_t index = 0;
    for (uint8_t suit = 0; suit < 4; ++suit) {
        for (uint8_t rank = 1; rank <= 13; ++rank) {
            deck->cards[index].rank = rank;
            deck->cards[index].suit = (BlackJackSuit)suit;
            ++index;
        }
    }
    srand(seed);
    for (uint32_t i = BLACK_JACK_DECK_SIZE - 1; i > 0; --i) {
        uint32_t j = (uint32_t)rand() % (i + 1);
        BlackJackCard temporary = deck->cards[i];
        deck->cards[i] = deck->cards[j];
        deck->cards[j] = temporary;
    }
    deck->next_card = 0;
}

uint8_t BlackJack_DeckDraw(BlackJackDeck *deck, BlackJackCard *card)
{
    if (deck == 0 || card == 0 || deck->next_card >= BLACK_JACK_DECK_SIZE)
        return 0;
    *card = deck->cards[deck->next_card++];
    return 1;
}

uint8_t BlackJack_DeckRemaining(const BlackJackDeck *deck)
{
    if (deck == 0 || deck->next_card >= BLACK_JACK_DECK_SIZE) return 0;
    return BLACK_JACK_DECK_SIZE - deck->next_card;
}

uint8_t GetHandScore(const Hand *hand)
{
    uint8_t total = 0, aces = 0;
    for (uint8_t i = 0; i < hand->count; ++i) {
        uint8_t rank = hand->cards[i].rank;
        if (rank == 1) { total += 11; ++aces; }
        else total += rank > 10 ? 10 : rank;
    }
    while (total > 21 && aces) { total -= 10; --aces; }
    return total;
}

uint8_t IsBlackjack(const Hand *hand)
{
    return hand->count == 2 && GetHandScore(hand) == 21;
}

void CenterText(uint8_t row, const char *text)
{
    size_t length = strlen(text);
    tft_printc(length < char_max_x ? (char_max_x - length) / 2 : 0, row, text);
}

void DrawCards(const Hand *hand)
{
    /* Up to six overlapping cards per row, two rows on the 128x160 TFT. */
    for (uint8_t row = 0; row < 2; ++row) {
        uint8_t first_card = row * 6;
        if (first_card >= hand->count) break;
        uint8_t card_count = hand->count - first_card;
        if (card_count > 6) card_count = 6;
        int16_t screen_width = char_max_x * CHAR_WIDTH;
        int16_t spacing = (screen_width - 8 - TFT_CARD_WIDTH) /
                          (card_count > 1 ? card_count - 1 : 1);
        if (spacing > TFT_CARD_WIDTH + 2) spacing = TFT_CARD_WIDTH + 2;
        int16_t x = (screen_width - (TFT_CARD_WIDTH + (card_count - 1) * spacing)) / 2;
        for (uint8_t i = 0; i < card_count; ++i) {
            BlackJackCard card = hand->cards[first_card + i];
            tft_draw_card(x + i * spacing, 48 + row * 36, card.rank, card.suit);
        }
    }
}
