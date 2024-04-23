#ifndef LISTING
#define LISTING

#include "header.h"

typedef struct __attribute__((__packed__)) Listing {
  char perms[11];
  char owner[23];
  char size[9];
  char mtime[17];
  char name[255];
} Listing;

void print_info(Header *h, int verbose);

#endif
  
