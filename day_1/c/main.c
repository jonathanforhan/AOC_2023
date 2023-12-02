#include <assert.h>
#include <ctype.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

/// @brief import a file to buffer
/// this function allows querying file len when buf is NULL because it's not this function's
/// job to alloc memory
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
    rewind(fp);

    if (buf == NULL) {
        goto abort;
    }

    if (fread(buf, sizeof(char), fsize, fp) != (size_t)fsize) {
        fprintf(stderr, "Possible missing data!\n");
    }

abort:
    fclose(fp);
    return fsize;
}

/// @brief sum the encoded input doc
/// we know that it must be a two digit number which simplifies things
/// @param doc str must be NULL terminated
/// @return sum
uint32_t sum_document_part_one(const char* doc) {
    uint32_t sum = 0;
    size_t i = 0;
    int32_t d1 = -1;
    int32_t d2 = -1;
    char c = '\0';

    do {
        c = doc[i];

        if (isdigit(c)) {
            if (d1 < 0) d1 = c - '0';
            else        d2 = c - '0';
        } else if (c == '\n') {
            assert(d1 >= 0);

            d2 = d2 >= 0 ? d2 : d1;
            sum += d1 * 10 + d2;
            d1 = d2 = -1;
        }

        i++;
    } while (c);

    return sum;
}


/// @brief sum the encoded input doc
/// in part two we must consider 'one' 'two' ... 'nine' as valid numbers
/// @param doc str must be NULL terminated
/// @return sum
uint32_t sum_document_part_two(const char* doc) {
    uint32_t sum = 0;
    size_t i = 0;
    size_t j = 0;
    int32_t d1 = -1;
    int32_t d2 = -1;
    char c = '\0';

    const char *digits[] = {
        "zero",
        "one",
        "two",
        "three",
        "four",
        "five",
        "six",
        "seven",
        "eight",
        "nine",
    };

    do {
        c = doc[i];

        if (isdigit(c)) {
            if (d1 < 0) d1 = c - '0';
            else        d2 = c - '0';
        } else if (c == '\n') {
            assert(d1 >= 0);

            d2 = d2 >= 0 ? d2 : d1;
            sum += d1 * 10 + d2;

            d1 = d2 = -1;
        } else {
            for (j = 0; j < 10; j++) {
                if (strncmp(doc + i, digits[j], strlen(digits[j])) == 0) {
                    if (d1 < 0) d1 = j;
                    else        d2 = j;
                    break;
                }
            }
        }

        i++;
    } while (c);

    return sum;
}

int main(void) {
    char *buf = NULL;
    ssize_t len;

    len = import("../input.txt", NULL);
    assert(len > 0);
    buf = calloc(len + 1, sizeof(char));
    assert(buf);

    import("../input.txt", buf);

    uint32_t sum = sum_document_part_one(buf);
    printf("Part 1 sum: %d\n", sum);

    sum = sum_document_part_two(buf);
    printf("Part 2 sum: %d\n", sum);

    free(buf);
    return 0;
}
