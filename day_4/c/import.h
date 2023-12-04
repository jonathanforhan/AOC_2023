#ifndef AOC_IMPORT_H
#define AOC_IMPORT_H

#include <stdio.h>

/// @brief import a file to buffer
/// @param path path of file
/// @param buf this is the buffer to file. If NULL the function just returns the file length
/// @return file length
ssize_t import(const char* path, char* buf);

#endif // AOC_IMPORT_H
