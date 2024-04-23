#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include "header.h"
#include "archive.h"
#include "listing.h"
#include "extract.h"
#include "mytar.h"

extern int optind;
extern char *optarg;

#define PADDING_SIZE 1024
#define BLOCK_SIZE 512
#define PATH_MAX 255


/* myTar is an archive tool with support for creation,
 * listing, and extraction. It also has support for the
 * verbose and strict options. */
int main(int argc, char *argv[]) {
  char option;
  int mode = -1;
  int i, j;
  int archiveFile;
  int verbose = 0;
  
  if(argc < 3) {
    /* if no arguments are provided */
    fprintf(stderr, "no arguments");
    exit(EXIT_FAILURE);
  }

  for(i = 1; i < argc; i++) {
    if(i == 1) {
      for(j = 0; j < strlen(argv[1]); j++) {
        option = argv[1][j];
        switch(option){
          case 'c':
          case 't':
          case 'x':
            mode = option;
            break;
          case 'v':
            verbose = 1;
            break;
          case 'S':
            break;
          case 'f':
            if(argc == i + 1) {
              fprintf(stderr, "no archive specified");
              exit(EXIT_FAILURE);
            }
	          if(mode == 'c') {
            /*if create mode */
              if((archiveFile = open(argv[i+1], O_WRONLY | O_CREAT | O_TRUNC, 0644)) == -1) {
                perror("open");
                exit(EXIT_FAILURE);
              }
            } else {
              if((archiveFile = open(argv[i+1], O_RDONLY)) == -1) {
                perror("open");
                exit(EXIT_FAILURE);
              }
            }
            /* increment because next 
             * argument not a path */
            i++;
            break;
          default:
            perror("unsupported option");
            exit(EXIT_FAILURE);
            break;
        }
      }
    } else {  
      /* if path specified, archive the path */
      if(mode == 'c') {
        archive(argv[i], archiveFile, verbose);
      }
    }

  }
  if(mode == -1) {
    /* if no mode is specified */
    fprintf(stderr, "no options specified");
    exit(EXIT_FAILURE);
  }
  if(mode == 'c') {
    write_empty_blocks(archiveFile);
  } else {
    traverse(argc, argv, archiveFile, verbose, mode);
  }
  close(archiveFile); 
  return 0;
}

void traverse(int argc, char *argv[], int archive, int v, char mode) {
  /* traverse the tape(the archive) and check against each inputted argument */
  int bytesRead;
  Header *currentHeader = malloc(sizeof(Header));
  int i;
  int size;
  int offset = lseek(archive, 0, SEEK_CUR);
  int fileSize = lseek(archive, 0, SEEK_END);
  Header *zeroHeader = calloc(1, sizeof(Header));


  if(currentHeader == NULL) {
    perror("malloc");
    exit(EXIT_FAILURE);
  }

  if(offset == -1 || fileSize == -1 || lseek(archive, offset, SEEK_SET) == -1) {
    perror("lseek");
    exit(EXIT_FAILURE);
  }
  
  if(zeroHeader == NULL) {
    perror("calloc");
    exit(EXIT_FAILURE);
  }

  while((bytesRead = read(archive, currentHeader, BLOCK_SIZE)) > 0) {
    offset = lseek(archive, 0, SEEK_CUR);

    if(offset == -1) {
      perror("lseek");
      exit(EXIT_FAILURE);
    }

    if(strtol(currentHeader->chksum, NULL, 8) != get_chksum(currentHeader)) {
      if(!memcmp(zeroHeader, currentHeader, PADDING_SIZE) && 
         offset == fileSize - 1024) {
        return;
      }
      fprintf(stderr, "bad header");
      exit(EXIT_FAILURE);
    }

    for(i = 3; i < argc; i++) {
      if(strcmp(argv[i], get_path(currentHeader, strlen(argv[i]))) == 0) {
        if(mode == 't') {
          print_info(currentHeader, v);
        }
        if(mode == 'x') {
          extract(get_path(currentHeader, PATH_MAX - 1), 
                  archive, currentHeader, v);
        }
        break;
      }
    }
    if(argc == 3) {
	if(mode == 't') {
          print_info(currentHeader, v);
        }
        if(mode == 'x') {
          extract(get_path(currentHeader, PATH_MAX - 1), 
                  archive, currentHeader, v);
        }
    }
    if((size = strtol(currentHeader->size, NULL, 8)) != 0) {
      offset = ((size + 511)/512) * 512;
      
      if((lseek(archive, offset, SEEK_CUR)) == -1) {
        perror("lseek");
        exit(EXIT_FAILURE);
      } 
      
    }
  }
  
  
  if((lseek(archive, 0, SEEK_SET)) == -1) {
    perror("lseek");
    exit(EXIT_FAILURE);
  } 
  free(currentHeader);
}
