#include <stdio.h>
#include <fcntl.h>
#include <unistd.h>
#include <string.h>
#include <stdlib.h>
#include "header.h"
#include "extract.h"

#define BLOCK_SIZE 512
#define PATH_MAX 255

void extract(char *path, int archiveFd, Header *h, int v) {
  /* Extracts directories and files from the archive based
   * on the path specified. Creates directories as it goes. */
  struct stat pathStat;
  char currPath[PATH_MAX] = "";
  int fd;
  char buffer[BLOCK_SIZE];
  int bytesRead = 0;
  int mode = strtol(h->mode, NULL, 8);
  int perms = 0666;
  int size;
  int i = 0;
  int j = 0;
  off_t offset = lseek(archiveFd, 0, SEEK_CUR);
  
  /* Create the directories leading to path */
  while(path[i] != '\0' && i < PATH_MAX) {
    if(path[i] == '/') {
      if(stat(currPath, &pathStat) != 0) {
        if(mkdir(currPath, 0777) == -1) {
          perror("mkdir");
          exit(EXIT_FAILURE);
        }
      }
    }
    currPath[j] = path[i];
    i++;
    j++;
  }
  if(mode & S_IXUSR || mode & S_IXGRP || mode & S_IXOTH) {
    perms = 0777;
  }
  if(v) {
    printf("%s\n", path);
  }
  if(h->typeflag[0] == '5') {
    /* If directory */
    if(stat(path, &pathStat) == 0) {
      /* If directory already exists */
      return;
    }
    /* If directory doesn't exist */
    if(mkdir(path, perms) == -1) {
      perror("mkdir");
      exit(EXIT_FAILURE);
    }
    return;
  }
  if(h->typeflag[0] == '0') {
    /* if reg file */
    fd = open(path, O_CREAT | O_WRONLY | O_TRUNC, perms);
    while((bytesRead += read(archiveFd, buffer, BLOCK_SIZE)) 
           < strtol(h->size, NULL, 8)) {
      
      if(write(fd, buffer, BLOCK_SIZE) == -1) {
        perror("write");
        exit(EXIT_FAILURE);
      }
    
    }
    size = strtol(h->size, NULL, 8);


    if(write(fd, buffer, (size < BLOCK_SIZE)?(size):(size%BLOCK_SIZE)) == -1) {
      perror("write");
      exit(EXIT_FAILURE);
    }
    
    
    close(fd);
    if(lseek(archiveFd, offset, SEEK_SET) == -1) {
      perror("lseek");
      exit(EXIT_FAILURE);
    }
  }
  if(h->typeflag[0] == '2') {
    /* if symlink */
    symlink(h->linkname, path);
  }

}
  
