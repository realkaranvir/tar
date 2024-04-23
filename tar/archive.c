#include <stdio.h>
#include <unistd.h>
#include <stdlib.h>
#include <string.h>
#include <fcntl.h>
#include <dirent.h>
#include <errno.h>
#include "header.h"
#include "archive.h"
#include "listing.h"

#define PADDING_SIZE 1024
#define BLOCK_SIZE 512
#define PATH_MAX 4096

void archive(const char *path, int outputFile, int verboseFlag) {
  Header *currentHeader = create_header();
  struct dirent* entry;
  DIR *dir = opendir(path);
  char currPath[PATH_MAX] = "";
  struct stat entryStat;
  int inputFile;
  
  if(dir == NULL) {
    perror("opendir");
    exit(EXIT_FAILURE);
  }
  
  /* write the first element out */ 
  snprintf(currPath, PATH_MAX, "%s", path);
  if(stat(currPath, &entryStat) == -1) {
    printf("file: %s failed to read, continuing\n", path);
    return;
  }
  if(S_ISDIR(entryStat.st_mode)) {
    /* if path starts with a directory, write header */
    snprintf(currPath, PATH_MAX, "%s/", path);
    populate_header(currentHeader, currPath);
    
    if(write(outputFile, currentHeader, BLOCK_SIZE) == -1) {
      perror("write");
      exit(EXIT_FAILURE);
    }

    if(verboseFlag) {
      print_info(currentHeader, 0); 
    }
    free(currentHeader); 
    currPath[strlen(currPath) - 1] = '\0';
  } else {
    /* if path is a file, write file and exit */
    inputFile = open(currPath, O_RDONLY);
    
    if(inputFile == -1) {
      perror("open error");
      exit(EXIT_FAILURE);
    }
    populate_header(currentHeader, currPath);
    if(verboseFlag) {
      print_info(currentHeader, 0); 
    }
    write_file(currentHeader, inputFile, outputFile);
    return;
  } 

  

  while((entry = readdir(dir)) != NULL) {
    /* traverse through path and archive directories and files */
    if(strcmp(entry->d_name, ".") == 0 || strcmp(entry->d_name, "..") == 0) {
      continue;
    }
    
    snprintf(currPath, PATH_MAX, "%s/%s", path, entry->d_name);
    if(stat(currPath, &entryStat) == -1) {
      perror("stat");
      exit(EXIT_FAILURE);
    }

    currentHeader = create_header();

    if(S_ISDIR(entryStat.st_mode)) {
      archive(currPath, outputFile, verboseFlag);
    
    } else {
      inputFile = open(currPath, O_RDONLY);

      if(inputFile == -1) {
        perror("error opening file");
        exit(EXIT_FAILURE);
      }
      
      populate_header(currentHeader, currPath);
      if(verboseFlag) {
        print_info(currentHeader, 0); 
      }
      write_file(currentHeader, inputFile, outputFile);
    }
  }
  if(errno != 0) {
    perror("Error reading the directory");
    closedir(dir);
    exit(EXIT_FAILURE);
  }
  closedir(dir);
}


void write_file(Header *h, int inputFile, int outputFile) {
  /* writes a file's header and its contents to the archive */
  int bytesRead;
  int bytesWritten;
  int numPadding;
  char *buffer = (char*)calloc(BLOCK_SIZE, sizeof(char));

  if(buffer == NULL) {
    perror("calloc");
    exit(EXIT_FAILURE);
  }

  if((bytesWritten = write(outputFile, h, BLOCK_SIZE)) == -1) {
    perror("write");
    exit(EXIT_FAILURE);
  }

  while((bytesRead = read(inputFile, buffer, BLOCK_SIZE)) > 0) {
    if((bytesWritten = write(outputFile, buffer, bytesRead)) == -1) {
      perror("write");
      exit(EXIT_FAILURE);
    }
    
    if((bytesWritten % BLOCK_SIZE) != 0) {
      numPadding = BLOCK_SIZE - (bytesWritten % BLOCK_SIZE);
      memset(buffer, 0, BLOCK_SIZE);
      write(outputFile, buffer, numPadding);

    }
    
  }
  if(bytesRead == -1) {
    perror("read");
    exit(EXIT_FAILURE);
  }
  
  close(inputFile);
  free(buffer);
  free(h);
}

void write_empty_blocks(int outputFile) {
  /* writes 2 empty blocks to signify end of archive */
  char buffer[PADDING_SIZE] = {0};
  if(write(outputFile, buffer, PADDING_SIZE) == -1) {
    perror("write");
    exit(EXIT_FAILURE);
  }
}

