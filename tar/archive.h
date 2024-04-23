#ifndef ARCHIVE
#define ARCHIVE

#include "header.h"

void archive(const char *path, int outputFile, int verboseFlag);

void write_file(Header *h, int inputFIle, int outputFile);

void write_empty_blocks(int outputFile);

#endif
