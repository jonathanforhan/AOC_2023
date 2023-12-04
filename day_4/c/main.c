#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "import.h"

// offset of the metadata tag on the cards
#define INPUT_OFFSET 9

/// @brief evaluate winning state of a scratcher
/// @param s NULL terminated string Card N: N N N | N N N N N N \0'
/// @return the score
uint32_t evaluate_card(const char *s) {
    char *scpy, *tok, *saveptr;
    uint32_t score = 0;
    uint16_t winners[10], tries[25];
    bool winning_numbers = true;
    size_t i = 0;
    size_t j = 0;

    scpy = calloc(strlen(s) + 1, sizeof(*s));
    assert(scpy);
    strcpy(scpy, s);

    tok = strtok_r(scpy + INPUT_OFFSET, " ", &saveptr);

    while (tok != NULL) {
        if (*tok == '|') {
            winning_numbers = false;
            i = 0;
        } else {
            uint16_t val = (uint16_t)strtoul(tok, NULL, 0);
            if (winning_numbers)
                winners[i] = val;
            else
                tries[i] = val;
            i++;
        }
        tok = strtok_r(NULL, " ", &saveptr);
    }

    for (i = 0; i < 10; i++) {
        for (j = 0; j < 25; j++) {
            if (tries[j] == winners[i]) {
                score = score ? score << 1 : 1;
                break;
            }
        }
    }

    free(scpy);

    return score;
}

/// @brief split cards by nl and sum the winning scores of each row
/// @param buf char buffer of scratcher data
/// @param count number of chars in buf
/// @return score
uint32_t evaluate_cards(const char *buf, size_t count) {
    char *s, *tok, *saveptr;
    uint32_t score = 0;

    s = calloc(count + 1, sizeof(*buf));
    assert(s);
    strncpy(s, buf, count);

    tok = strtok_r(s, "\n", &saveptr);

    while (tok != NULL) {
        score += evaluate_card(tok);
        tok = strtok_r(NULL, "\n", &saveptr);
    }

    free(s);

    return score;
}

/// @brief evaluate winning state of a scratcher tracked now as the number of times won
/// @param s NULL terminated string Card N: N N N | N N N N N N \0'
/// @return the score
uint32_t evaluate_card_part_2(const char *s) {
    char *scpy, *tok, *saveptr;
    uint32_t score = 0;
    uint16_t winners[10], tries[25];
    bool winning_numbers = true;
    size_t i = 0;
    size_t j = 0;

    scpy = calloc(strlen(s) + 1, sizeof(*s));
    assert(scpy);
    strcpy(scpy, s);

    tok = strtok_r(scpy + INPUT_OFFSET, " ", &saveptr);

    while (tok != NULL) {
        if (*tok == '|') {
            winning_numbers = false;
            i = 0;
        } else {
            uint16_t val = (uint16_t)strtoul(tok, NULL, 0);
            if (winning_numbers)
                winners[i] = val;
            else
                tries[i] = val;
            i++;
        }
        tok = strtok_r(NULL, " ", &saveptr);
    }

    for (i = 0; i < 10; i++) {
        for (j = 0; j < 25; j++) {
            if (tries[j] == winners[i]) {
                score++;
                break;
            }
        }
    }

    free(scpy);

    return score;
}

/// @brief slide our stack by one and replace the last val with 1
/// [1, 2, 3, 4, 5, 6, 7, 8, 9] -> [2, 3, 4, 5, 6, 7, 8, 9, 1]
/// @param stack stack of 10
void slide_stack(uint32_t stack[10]) {
    int i;
    for (i = 0; i < 10 - 1; i++)
        stack[i] = stack[i + 1];
    stack[i] = 1;
}

/// @brief split cards by nl and track the wins
/// if Card 1 get 4 wins then we earn 4 more scratches, a Card 2, 3, 4, and 5 (totals 4)
/// we then track these cards as they snowball to see the total number of cards gotten
/// @param buf char buffer of scratcher data
/// @param count number of chars in buf
/// @return number of cards
uint32_t evaluate_cards_part_2(const char *buf, size_t count) {
    char *s, *tok, *saveptr;
    uint32_t score = 0;
    size_t i;

    uint32_t stack[10];
    for (i = 0; i < 10; i++)
        stack[i] = 1;

    s = calloc(count + 1, sizeof(*buf));
    assert(s);
    strncpy(s, buf, count);

    tok = strtok_r(s, "\n", &saveptr);

    while (tok != NULL) {
        uint32_t val = evaluate_card_part_2(tok);
        uint32_t n_cards = stack[0];
        slide_stack(stack);
        for (i = 0; i < val; i++) {
            stack[i] += n_cards;
        }
        score += n_cards;

        tok = strtok_r(NULL, "\n", &saveptr);
    }

    free(s);

    return score;
}

int main(void) {
    char *buf;
    ssize_t len;

    len = import("input.txt", NULL);
    assert(len > 0);

    buf = calloc(len + 1, sizeof(char));
    import("input.txt", buf);
    assert(buf);

    printf("Score part 1: %u\n", evaluate_cards(buf, len));
    printf("Score part 2: %u\n", evaluate_cards_part_2(buf, len));

    free(buf);

    return 0;
}
