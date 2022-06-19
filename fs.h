// meta info of file system
// number of inodes / disc blocks
// size of disc blocks
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BLOCKSIZE 512

struct superblock {
    int num_blocks;
    int num_inodes;
    int size_blocks;
};

struct inode {
    int size;
    int first_block;
    int fd;
    char name[20];
};

struct disc_block {
    int next_block_num;
    char data[BLOCKSIZE];
};

struct mydirent { 
    int size;
    int fds[5];
    char d_name[20];
};

struct myDIR { 
    int path;
    char * d_name;
};

struct myopenfiles {
    int fd;
    int indx;
};

void mymkfs();
int mymount(const char *, const char *, const char *, unsigned long, const void *); // load a fs
int myopen(const char *, int);
int myclose(int);
size_t myread(int, void *, size_t);
size_t mywrite(int, const void *, size_t);
int mylseek(int, int, int);
int myopendir(const char *);
struct mydirent *myreaddir(int);
int myclosedir(struct myDIR *);

void create_fs(); // init fs
void sync_fs(); // write the fs
int allocate_file(int, const char *); //return filenumber
void set_filesize(int, int);
void write_byte(int, int, char *);
void print_fs(); // for debugging