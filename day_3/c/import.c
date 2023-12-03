#include "import.h"

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
