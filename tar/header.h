#ifndef HEADER
#define HEADER

#include <arpa/inet.h>
#include <sys/stat.h>

typedef struct __attribute__((__packed__)) Header{
  char name[100];
  char mode[8];
  char uid[8];
  char gid[8];
  char size[12];
  char mtime[12];
  char chksum[8];
  char typeflag[1];
  char linkname[100];
  char magic[6];
  char version[2];
  char uname[32];
  char gname[32];
  char devmajor[8];
  char devminor[8];
  char prefix[155];
  char padding[12];
} Header;

Header *create_header(void);

void populate_header(Header *h, char *filepath);

void populate_type_and_link(Header *h, struct stat *fileStat, char *filepath);

void write_header(Header *h, const char *outputFile);

int get_chksum(Header *h);

uint32_t extract_special_int(char *where, int len);

int insert_special_int(char *where, size_t size, int32_t val);

char *get_path(Header *h, int length);
#endif
