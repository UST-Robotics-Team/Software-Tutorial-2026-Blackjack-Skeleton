#ifndef UART_COMMANDS_H
#define UART_COMMANDS_H
#include <stdint.h>
#include <string.h>

#define COMMAND_SIZE 256

/* Provided UART plumbing. The interrupt collects a newline-terminated string;
   the application handles it in the main loop, then clears message_ready. */
typedef struct {
    char building[COMMAND_SIZE];
    char message[COMMAND_SIZE];
    uint16_t character_count;
    uint8_t discard_line;
    volatile uint8_t message_ready;
} CommandBuffer;

static inline void CollectCommandByte(CommandBuffer *buffer, uint8_t byte)
{
    if (byte == '\r') return;
    if (byte == '\n') {
        if (buffer->character_count && !buffer->discard_line && !buffer->message_ready) {
            buffer->building[buffer->character_count] = '\0';
            strcpy(buffer->message, buffer->building);
            buffer->message_ready = 1;
        }
        buffer->character_count = buffer->discard_line = 0;
    } else if (!buffer->discard_line) {
        if (byte < 32 || byte > 126 || buffer->character_count >= COMMAND_SIZE - 1)
            buffer->discard_line = 1;
        else buffer->building[buffer->character_count++] = byte;
    }
}
#endif
