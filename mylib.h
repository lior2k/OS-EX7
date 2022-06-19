#include "fs.h"

struct myFILE {
    char *name;
    // char mode[3];
    int fd;
    int pointer;
};

struct myFILE *myfopen(const char *, const char *);
int myfclose(struct myFILE *);
size_t myfread(void *ptr, size_t, size_t, struct myFILE *);
size_t myfwrite(const void *, size_t, size_t, struct myFILE *);
int myfseek(struct myFILE *, long, int);
int myfscanf(struct myFILE *, const char *, ...);
int myfprintf(struct myFILE *, const char *, ...);