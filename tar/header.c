#include <stdio.h>
#include <string.h>
#include <sys/stat.h>
#include <stdlib.h>
#include <unistd.h>
#include <pwd.h>
#include <grp.h>
#include <fcntl.h>
#include <arpa/inet.h>
#include "header.h"


#define NAME_MAX 100
#define PATH_MAX 255
#define REG_FILE '0'
#define REG_FILE_ALT '\0'
#define SYM_LINK '2'
#define DIRECTORY '5'
#define HEADER_SIZE 512


Header *create_header(void) {
  /* initializes header */
  Header *h = (Header *)calloc(1, sizeof(Header));
  if(h == NULL) {
    perror("calloc");
    exit(EXIT_FAILURE);
  }
  
  return h;
}

void populate_header(Header *h, char *filePath) {
  /* populates header based on the filepath */
  struct stat fileStat;
  struct passwd *user = NULL;
  struct group *group = NULL;
  int i;
  unsigned int sum = 0;
  if(stat(filePath, &fileStat) == -1) {
    perror("lstat");
    exit(EXIT_FAILURE);
  }
  
  if(strlen(filePath) > NAME_MAX) {
    /* if path longer than 100 bytes, split between
     * name and prefix, else all goes into name */
    for (i = strlen(filePath) - NAME_MAX - 1; i >= 0; i++) {
        if (filePath[i] == '/') {
            break;
        }
    }
    memcpy((h->name), filePath + i + 1, strlen(filePath) - i - 1);
    memcpy((h->prefix), filePath, i);
  } else {
    memcpy((h->name), filePath, strlen(filePath));
  }
  /* copy octal representation of mode to h->mode */
  sprintf(h->mode, "%07o", fileStat.st_mode & 0777);
  
  if(fileStat.st_uid > 07777777) {
    /* if uid is too long to store in octal */
    if(insert_special_int(h->uid, sizeof(h->uid), fileStat.st_uid) != 0) {
      fprintf(stderr, "uid error\n");
      exit(EXIT_FAILURE);
    }
  }
  else {
    sprintf(h->uid, "%07o", fileStat.st_uid);
  }
  /* store gid */
  sprintf(h->gid, "%07o", fileStat.st_gid);
  
  /* store size of file */
  sprintf(h->size, "%011o", fileStat.st_size);
  
  /* store mtime of file */
  sprintf(h->mtime, "%011o", fileStat.st_mtime);

  /* store type and link of file */
  populate_type_and_link(h, &fileStat, filePath);

  sprintf(h->magic, "ustar");

  memcpy(h->version, "00", 2);

  user = getpwuid(fileStat.st_uid);
  if(user == NULL) {
    perror("getpwuid");
    exit(EXIT_FAILURE);
  }
  group = getgrgid(fileStat.st_gid);
  if(group == NULL) {
    perror("getgrid");
    exit(EXIT_FAILURE);
  }

  snprintf(h->uname, sizeof(h->uname), "%s", user->pw_name);
  snprintf(h->gname, sizeof(h->gname), "%s", group->gr_name);
  
  sum = get_chksum(h);

  sprintf(h->chksum, "%07o", sum);
  
}

int get_chksum(Header *h) {
  unsigned char *chksum = NULL;
  int sum = 0;
  int i = 0;
  chksum = (unsigned char*)h;
  for(i = 0; i < HEADER_SIZE; i++) {
    if(i < 148 || i > 155) {
      sum += chksum[i];
    } else {
      sum += ' ';
    }
  }
  return sum;
}



void populate_type_and_link(Header *h, struct stat *fileStat, char *filePath) {
  char buffer[NAME_MAX] = {0};
  int bytesRead = 0;

  if(S_ISREG(fileStat->st_mode)) {
    h->typeflag[0] = REG_FILE;
    return;
  }
  
  if(S_ISLNK(fileStat->st_mode)) {
    h->typeflag[0] = SYM_LINK;
    sprintf(h->size, "00000000000");
    bytesRead = readlink(filePath, h->linkname, sizeof(buffer));
    if(bytesRead != 100) {
      (h->linkname)[bytesRead] = '\0';
    }
    return;
  }
  
  if(S_ISDIR(fileStat->st_mode)) {
    h->typeflag[0] = DIRECTORY;
    sprintf(h->size, "00000000000");
    return;
  }
  
  h->typeflag[0] = REG_FILE_ALT;
  return;
}

char *get_path(Header *h, int length) {
  /* Takes a header and returns the path
   * truncated to a certain length.
   * Useful for comparing paths */
  char *path = malloc(PATH_MAX);
  if(h->prefix[0] != '\0') {
    snprintf(path, length + 1, "%s/%s", h->prefix, h->name);
  } else {
    if(length > NAME_MAX){
      length = NAME_MAX;
    }
    snprintf(path, length + 1, "%s", h->name);
  }
  return path;
}


uint32_t extract_special_int(char *where, int len) {
  /* For interoperability with GNU tar. GNU seems to
  * set the high–order bit of the first byte, then
  * treat the rest of the field as a binary integer
  * in network byte order.
  * I don’t know for sure if it’s a 32 or 64–bit int,
  * but for * this version, we’ll only support 32. (well, 31)
  * returns the integer on success, –1 on failure.
  * In spite of the name of htonl(), it converts int32 t */
  int32_t val = -1;
  if ((len >= sizeof(val)) && (where[0] & 0x80)) {
  /* the top bit is set and we have space * extract the last four bytes */
    val = *(int32_t *)(where + len - sizeof(val));
    val = ntohl(val); /* convert to host byte order */ }
  return val; 
}
int insert_special_int(char *where, size_t size, int32_t val) { 
 /* For interoperability with GNU tar. GNU seems to
  * set the high–order bit of the first byte, then
  * treat the rest of the field as a binary integer
  * in network byte order.
  * Insert the given integer into the given field
  * using this technique. Returns 0 on success, nonzero * otherwise
  */
  int err = 0;
  if(val < 0||(size < sizeof(val))){
  /* if it’s negative, bit 31 is set and we can’t use the flag
  * if len is too small, we can’t write it. * done.
  */
    err++;
  } else {
  /* game on....*/
    memset(where, 0, size);
    *(int32_t *)(where + size - sizeof(val)) = htonl(val); /* place the int */ 
    *where |= 0x80; /* set that high–order bit */
  } 
  return err; 
}
