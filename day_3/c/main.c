#include <ctype.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <string.h>
#include "./import.h"

/// @brief query width of a char buf
/// @param buf NULL terminated buffer
/// @return width
size_t graph_width(const char *buf) {
    size_t i = 0;
    char c = *buf;

    while (c != '\n' && c != '\0') {
        i++;
        c = buf[i];
    }
    return i + 1; // account of newline
}

/// @brief query if symbol is valid in graph contexts
/// @param c char
/// @return validity
bool is_symbol(char c) {
    return c != '.' && !isdigit(c);
}

/// @brief sum the graph
/// @param char buf
/// @param how many elems
/// @param width of graph
/// @return sum
uint32_t sum(const char *buf, size_t count, size_t width) {
    uint32_t sum = 0;
    uint32_t curr = 0;
    bool valid = false;
    size_t i = 0;
    char c;

    // if bound is true then that attrib is ignored
    // for example bound.above == true we don't check above the node
    struct Bound {
        bool above;
        bool below;
        bool left;
        bool right;
    } bound;

    for (i = 0; i < count; i++) {
        c = buf[i];

        // if not digit reset
        if (!isdigit(c)) {
            if (valid) {
                sum += curr;
            }
            curr = valid = 0;
            continue;
        }

        curr = curr * 10 + (c - '0');

        // don't search if already valid
        if (valid)
            continue;

        bound = (struct Bound){
            .above = i < width,
            .below = i > count - width - 1,
            .left = (i % width) == 0,
            .right = (i % width) == width - 2, // account for nl
        };

        // query validity, this is branchless for readability

        // @ . .
        // . . .
        // . . .
        valid = valid || (!bound.above && !bound.left && is_symbol(buf[i - width - 1]));
        // . @ .
        // . . .
        // . . .
        valid = valid || (!bound.above && is_symbol(buf[i - width]));
        // . . @
        // . . .
        // . . .
        valid = valid || (!bound.above && !bound.right && is_symbol(buf[i - width + 1]));
        // . . .
        // @ . .
        // . . .
        valid = valid || (!bound.left && is_symbol(buf[i - 1]));
        // . . .
        // . . @
        // . . .
        valid = valid || (!bound.right && is_symbol(buf[i + 1]));
        // . . .
        // . . .
        // @ . .
        valid = valid || (!bound.below && !bound.left && is_symbol(buf[i + width - 1]));
        // . . .
        // . . .
        // . @ .
        valid = valid || (!bound.below && is_symbol(buf[i + width]));
        // . . .
        // . . .
        // . . @
        valid = valid || (!bound.below && !bound.right && is_symbol(buf[i + width + 1]));
    }

    return sum;
}

/// @brief give it a buffer and index at a gear. The offset is wear a part of a digit is
/// @param buf buffer of atleast size index
/// @param width maximum number of elements that compose number
/// @param index the current index of the gear
/// @param offset the offset of the digit to query
uint32_t buf_to_uint(const char *buf, size_t width, size_t index, size_t offset) {
    char *num_buf, *start;
    int32_t i;
    int32_t buf_offset = index + offset;
    int32_t num_buf_offset = buf_offset % width;

    assert(isdigit(buf[buf_offset]));

    // this is a slice of the row we are trying to determine the digit of like:
    // \0 \0 \0 '3' '2' '1' \0 \0 \0 ...
    num_buf = calloc(width + 1, sizeof(char));
    assert(num_buf);

    // slide right until hit non-digit
    for (i = 0; (num_buf_offset + i) < (int64_t)width && isdigit(buf[buf_offset + i]); i++) {
        num_buf[num_buf_offset + i] = buf[buf_offset + i];
    }

    // slide left until hit non-digit
    for (i = -1; (num_buf_offset + i) >= 0 && isdigit(buf[buf_offset + i]); i--) {
        num_buf[num_buf_offset + i] = buf[buf_offset + i];
    }

    start = &num_buf[num_buf_offset + i + 1]; // start of valid number
    uint32_t ret = strtoul(start, NULL, 10);

    free(num_buf);

    return ret;
}

/// @brief find gear ratio
/// a '*' is a gear if it's between exactly two numbers
/// we gotta go full graph
/// @param char buf
/// @param how many elems
/// @param width of graph
/// @return sum
uint32_t gear_ratio(const char *buf, size_t count, size_t width) {
    uint32_t sum = 0;
    uint32_t curr = 1;
    uint32_t n_around = 0; // how many numbers around gear
    size_t i = 0;
    char c;

    const size_t NW = -width - 1;  // northwest offset
    const size_t N  = -width;      // north offset
    const size_t NE = -width + 1;  // northeast offset
                                   //
    const size_t SW = width - 1;  // southwest offset
    const size_t S  = width;      // south offset
    const size_t SE = width + 1;  // southeast offset

    // if bound is true then that attrib is ignored
    // for example bound.above == true we don't check above the node
    struct Bound {
        bool above;
        bool below;
        bool left;
        bool right;
    } bound;

    for (i = 0; i < count; i++) {
        c = buf[i];

        if (c != '*') {
            continue;
        }

        bound = (struct Bound){
            .above = i < width,
            .below = i > count - width - 1,
            .left = (i % width) == 0,
            .right = (i % width) == width - 2, // account for nl
        };

        n_around = 0;
        curr = 1; // current ratio

        if (!bound.left && isdigit(buf[i - 1])) {
            n_around++;
            curr *= buf_to_uint(buf, width, i, -1);
        }
        if (!bound.right && isdigit(buf[i + 1])) {
            n_around++;
            curr *= buf_to_uint(buf, width, i, 1);
        }
        if (!bound.above) {
            // edge case (rest are exclusive)
            // @ . @
            // . . .
            // . . .
            if (!bound.left &&
                !bound.right &&
                isdigit(buf[i + NW])
                && isdigit(buf[i + NE])
                && !isdigit(buf[i + N]))
            {
                n_around += 2;
                curr *= buf_to_uint(buf, width, i, NW);
                curr *= buf_to_uint(buf, width, i, NE);
            // NW
            } else if (!bound.left && isdigit(buf[i + NW])) {
                n_around++;
                curr *= buf_to_uint(buf, width, i, NW);
            // N
            } else if (isdigit(buf[i + N])) {
                n_around++;
                curr *= buf_to_uint(buf, width, i, N);
            // NE
            } else if (!bound.right && isdigit(buf[i + NE])) {
                n_around++;
                curr *= buf_to_uint(buf, width, i, NE);
            }
        }
        if (!bound.below) {
            // edge case (rest are exclusive)
            // . . .
            // . . .
            // @ . @
            if (!bound.left &&
                !bound.right &&
                isdigit(buf[i + SW])
                && isdigit(buf[i + SE])
                && !isdigit(buf[i + S]))
            {
                n_around += 2;
                curr *= buf_to_uint(buf, width, i, SW);
                curr *= buf_to_uint(buf, width, i, SE);
            // SW
            } else if (!bound.left && isdigit(buf[i + SW])) {
                n_around++;
                curr *= buf_to_uint(buf, width, i, SW);
            // S
            } else if (isdigit(buf[i + S])) {
                n_around++;
                curr *= buf_to_uint(buf, width, i, S);
            // SE
            } else if (!bound.right && isdigit(buf[i + SE])) {
                n_around++;
                curr *= buf_to_uint(buf, width, i, SE);
            }
        }

        if (n_around == 2) {
            sum += curr;
        }
    }

    return sum;
}

int main(void) {
    char *buf;
    ssize_t len;

    len = import("input.txt", NULL);
    assert(len > 0);

    buf = calloc(len + 1, sizeof(char));
    import("input.txt", buf);
    assert(buf);

    size_t w = graph_width(buf);

    printf("sum: %u\n", sum(buf, len, w));
    printf("gear: %d\n", gear_ratio(buf, len, w));

    free(buf);

    return 0;
}
