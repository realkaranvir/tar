#include <stdio.h>
#include <stdlib.h>
#include <time.h>
#include <string.h>
#include <unistd.h>
#include "listing.h"
#include "header.h"

#define PATH_MAX 255
#define PREFIX_MAX 155
#define NAME_MAX 100
#define BLOCK_SIZE 512



void print_info(Header *h, int verbose) {
  /* Prints the info from the header. Takes
   * an int representing the verbose option and
   * the header to get info from. */
  struct tm *timeInfo;
  time_t mtime;
  int i = 0;
  char perm;
  char name[255];
  char owner[63];
  char mtimeConverted[18];

  if(h->name[0] == '\0') {return;}
  if(h->prefix[0] != '\0') {
    snprintf(name, PREFIX_MAX, "%s", h->prefix);
    if(name[strlen(name)] != '/' && h->name[0] != '/') {
      sprintf(name + strlen(name), "/");
    } 
    snprintf(name + strlen(name), NAME_MAX + 1, "%s", h->name);
  } else {
    snprintf(name, NAME_MAX + 1, "%s", h->name);
  }
  if(!verbose) {
    printf("%s\n", name);
    return;

  }     
  /* create and place perms string */
  if(h->typeflag[0] == '5') {
    printf("d");
  } else if(h->typeflag[0] == '2') {
    printf("l");
  } else {
    printf("-");
  }
  for(i = 0; i < 3; i++) {
    perm = h->mode[4 + i];
    switch(perm) {
      case '7':
        printf("rwx");
        break;
      case '6':
        printf("rw-");
        break;
      case '4':
        printf("r--");
        break;
      case '5':
        printf("r-x");
        break;
      case '3':
        printf("-wx");
        break;
      case '2':
        printf("-w-");
        break;
      case '1':
        printf("--x");
      default:
        printf("---");
    }
  }

  /* print owner string */
  sprintf(owner, " %s/%s ", h->uname, h->gname);
  printf("%-19s", owner);
  /* place size */
  printf("%8d ", strtol(h->size, NULL, 8));
  /* place mtime */
  mtime = strtol(h->mtime, NULL, 8);
  timeInfo = localtime(&mtime);
  strftime(mtimeConverted, sizeof(mtimeConverted), "%Y-%m-%d %H:%M", timeInfo);
  printf("%s ", mtimeConverted); 
  printf("%s\n", name);

  
}

