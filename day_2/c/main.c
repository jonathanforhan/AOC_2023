#include <assert.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define N_GAMES 100
#define MAX_RED 12
#define MAX_GREEN 13
#define MAX_BLUE 14

#define MAX(a, b) (a > b ? a : b)

/// @brief a Game will consist of multiple cube sets
typedef struct CubeSet {
    struct CubeSet* next;   ///< next Set of cubes NULL if final set
    uint32_t r;             ///< number of red cubes in set
    uint32_t g;             ///< number of green cubes in set
    uint32_t b;             ///< number of blue cubes in set
} CubeSet;

/// @brief provide nice struct for tokenizing input
typedef struct Token {
    const char *delim;
    char *tokptr;
    char *saveptr;
} Token;

/// @brief import a file to buffer
/// @param path path of file
/// @param buf this is the buffer to file. If NULL the function just returns the file length
/// @return file length
ssize_t import(const char* path, char* buf) {
    FILE* fp = NULL;
    ssize_t fsize = -1;

    fp = fopen(path, "r");
    if (fp == NULL) {
        fprintf(stderr, "File import error!\n");
        goto abort;
    }

    fseek(fp, 0L, SEEK_END);
    fsize = ftell(fp);
    if (fsize < 0) {
        fprintf(stderr, "File read error!\n");
        goto abort;
    }

    if (buf == NULL) {
        goto abort;
    }

    rewind(fp);

    if (fread(buf, sizeof(char), fsize, fp) != (size_t)fsize) {
        fprintf(stderr, "Possible missing data!\n");
    }

abort:
    fclose(fp);
    return fsize;
}

/// @brief build an array of cube lists with input data
void build_cube_set(const char *str, CubeSet cube_set[N_GAMES]) {
    Token sp = { .delim = " " };   // space token
    Token nl = { .delim = "\n" };  // newline token
    uint32_t nth_game = 0;
    uint32_t last_val;
    char *tokens;
    CubeSet *curr_set;

    tokens = malloc(strlen(str) + 1);
    assert(tokens);
    strcpy(tokens, str);
    
    nl.tokptr = strtok_r(tokens, nl.delim, &nl.saveptr);
    while (nl.tokptr != NULL) {
        curr_set = &cube_set[nth_game++];
        memset(curr_set, 0, sizeof(*curr_set));

        sp.tokptr = strtok_r(nl.tokptr, sp.delim, &sp.saveptr);

        // skip game tag and number
        sp.tokptr = strtok_r(NULL, sp.delim, &sp.saveptr);
        sp.tokptr = strtok_r(NULL, sp.delim, &sp.saveptr);

        while (sp.tokptr != NULL) {
            if (strncmp(sp.tokptr, "red", 3) == 0) {
                curr_set->r = last_val;
            } else if (strncmp(sp.tokptr, "blue", 4) == 0) {
                curr_set->b = last_val;
            } else if (strncmp(sp.tokptr, "green", 5) == 0) {
                curr_set->g = last_val;
            } else /* must be digit */ {
                // save this info for when we get a color tag
                last_val = strtoul(sp.tokptr, NULL, 10);
            }

            sp.tokptr = strtok_r(NULL, sp.delim, &sp.saveptr);
            // check for semicolon delim at end of last sp.tokptr
            if (sp.tokptr && *(sp.tokptr - 2) == ';') {
                // add a link to next child cube set
                // gets cleaned up in free_cube_set
                curr_set->next = calloc(1, sizeof(CubeSet));
                curr_set = curr_set->next;
                assert(curr_set);
            }
        }

        nl.tokptr = strtok_r(NULL, nl.delim, &nl.saveptr);
    }

    free(tokens);
}

/// @brief free cube set child, recursive
void free_child_cube_set(CubeSet *cube_set) {
    if (cube_set->next)
        free_child_cube_set(cube_set->next);

    free(cube_set);
}

/// @brief free cube set array
void free_cube_set(CubeSet cube_set[N_GAMES]) {
    uint32_t i;

    for (i = 0; i < N_GAMES; i++) {
        if (cube_set[i].next)
            free_child_cube_set(cube_set[i].next);
    }
}

/// @brief returns validity of child game, recursive
bool audit_cube_set_child(CubeSet *cube_set) {
    if (cube_set->r > MAX_RED ||
        cube_set->g > MAX_GREEN ||
        cube_set->b > MAX_BLUE
    ) return false;

    return cube_set->next
        ? audit_cube_set_child(cube_set->next)
        : true;
}

/// @brief return sum of all valid games
uint32_t audit_cube_set(CubeSet cube_set[N_GAMES]) {
    uint32_t sum = 0;

    for (uint32_t i = 0; i < N_GAMES; i++) {
        if (cube_set[i].r > MAX_RED ||
            cube_set[i].g > MAX_GREEN ||
            cube_set[i].b > MAX_BLUE
        ) continue;

        if (cube_set[i].next && audit_cube_set_child(cube_set[i].next)) {
            sum += i + 1; // adjust for zero index
        }
    }

    return sum;
}

/// @brief find power of cube sets
uint32_t power_cube_set(CubeSet cube_set[N_GAMES]) {
    uint32_t power = 0;
    uint32_t r_max = 0;
    uint32_t g_max = 0;
    uint32_t b_max = 0;
    CubeSet *curr_set;
    
    for (uint32_t i = 0; i < N_GAMES; i++) {
        curr_set = &cube_set[i];
        while (curr_set != NULL) {
            r_max = MAX(r_max, curr_set->r);
            g_max = MAX(g_max, curr_set->g);
            b_max = MAX(b_max, curr_set->b);
            curr_set = curr_set->next;
        }

        power += r_max * g_max * b_max;
        r_max = g_max = b_max = 0;
    }

    return power;
}

int main(void) {
    char *buf = NULL;
    ssize_t len;
    CubeSet games[N_GAMES];
    uint32_t sum, power;

    len = import("input.txt", NULL);
    assert(len > 0);
    buf = calloc(len + 1, sizeof(char));
    assert(buf);

    import("input.txt", buf);
    build_cube_set(buf, games);

    sum = audit_cube_set(games);
    printf("sum of valid games: %u\n", sum);

    power = power_cube_set(games);
    printf("power of games: %u\n", power);

    free_cube_set(games);

    free(buf);

    return 0;
}
