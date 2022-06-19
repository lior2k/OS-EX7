#include "fs.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

// source code: https://www.youtube.com/watch?v=n2AAhiujAqs

struct superblock sb;
struct inode *inodes;
struct disc_block *dbs;
struct myopenfiles openfiles[20];

int mymount(const char *source, const char *target,
                const char *filesystemtype, unsigned long mountflags, const void *data) {
    FILE *file;
    file = fopen("fs_data", "r");

    // superblock
    fread(&sb, sizeof(struct superblock), 1, file);

    for (int i = 0; i < sb.num_inodes; i++) {
        fread(&inodes[i], sizeof(struct inode), 1, file);
    }

    for (int i = 0; i < sb.num_blocks; i++) {
        fread(&dbs[i], sizeof(struct disc_block), 1, file);
    }

    fclose(file);
    return 0;
}

int myopen(const char *path, int flags) {
    char *str = (char*)malloc(sizeof(char)*strlen(path));
    strcpy(str, path);
    const char s[2] = "/";
    char *token = strtok(str, s);
    char current_path[strlen(path)];
    char previous_path[strlen(path)];
    while (token != NULL) {
        strcpy(previous_path, current_path);
        strcpy(current_path, token);
        token = strtok(NULL, s);
    }
    for (int i = 0; i < sb.num_inodes; i++) {
        if (!strcmp(inodes[i].name, current_path)) {
            openfiles[i].fd = i;
            openfiles[i].indx = 0;
            return i;
        }
    }
    int fd = allocate_file(1, current_path);
    int dirfd = myopendir(previous_path);
    struct mydirent *curr = myreaddir(dirfd);
    curr->fds[curr->size++] = fd;
    openfiles[fd].fd = fd;
    openfiles[fd].indx = 0;
    return fd;
}

int myclose(int myfd) {
    openfiles[myfd].fd = -1;
    return 0;
}

size_t myread(int myfd, void *buf, size_t count) {
    char *buffer = (char*)malloc(count + 1);
    for (int i = 0; i < count; i++) {
        buffer[i] = dbs[inodes[myfd].first_block].data[openfiles[myfd].indx++];
    }
    buffer[count] = '\0';
    bzero(buf,count);
    strncpy(buf, buffer, count);
    free(buffer);
    return count;
}

size_t mywrite(int myfd, const void *buf, size_t count) {
    write_byte(myfd, openfiles[myfd].indx, (char *) buf);
    openfiles[myfd].indx += (count);
    return openfiles[myfd].indx;
}

int mylseek(int myfd, int offset, int whence) {
    if (whence == SEEK_CUR) {
        openfiles[myfd].indx += offset;
    } else if (whence == SEEK_END) {
        openfiles[myfd].indx = inodes[myfd].size + offset;
    } else if (whence == SEEK_SET) {
        openfiles[myfd].indx = offset;
    }
    return openfiles[myfd].indx;
}

int myopendir(const char *name) {
    char *str = (char*)malloc(sizeof(char)*strlen(name));
    strcpy(str, name);
    char *token = strtok(str, "/");
    char current_path[strlen(name)];
    char previous_path[strlen(name)];
    while (token != NULL) {
        strcpy(previous_path, current_path);
        strcpy(current_path, token);
        token = strtok(NULL, "/");
    }
    int fd = -1;
     for (int i = 0; i < sb.num_inodes; i++) {
        if (!strcmp(inodes[i].name, current_path)) {
            fd = i;
            break;
        }
    }
    // didnt find fd
    if (fd == -1) {
        printf("fd not found");
        return -1;
    }
    struct mydirent *current_dir = (struct mydirent *) dbs[inodes[fd].first_block].data;
    current_dir->fds[current_dir->size++] = allocate_file(sizeof(struct mydirent), current_path);
    struct mydirent *dir = (struct mydirent*)malloc(sizeof(struct mydirent));
    dir->size = 0;
    return current_dir->fds[current_dir->size - 1];
}

struct mydirent *myreaddir(int dirp) {
    return (struct mydirent *) dbs[inodes[dirp].first_block].data;
}

int myclosedir(struct myDIR *dirp) {
    return 0;
}

int find_empty_inode() {
    int i;
    for(i = 0; i < sb.num_inodes; i++){
        if(inodes[i].first_block == -1){
            return i;
        }
    }
    return -1;
}

int find_empty_block(){
    for(int i = 0; i < sb.num_blocks; i++){
        if(dbs[i].next_block_num == -1){
            return i;
        }
    }
    return -1;
}

void shorten_file(int bn){
    int nn = dbs[bn].next_block_num;
    if (nn >= 0){
        shorten_file(nn);
    }
    dbs[bn].next_block_num = -1;
}

int get_block_num(int file, int offset){
    int togo = offset;
    int bn = inodes[file].first_block;
    while(togo > 0){
        bn = dbs[bn].next_block_num;
        togo--;
    }
    return bn;
}

void create_fs() {
    sb.num_inodes = 10;
    sb.num_blocks = 100;
    sb.size_blocks = sizeof(struct disc_block);

    inodes = malloc(sizeof(struct inode) * sb.num_inodes);
    for (int i = 0; i < sb.num_inodes; i++) {
        inodes[i].size = -1;
        inodes[i].first_block = -1;
        strcpy(inodes[i].name, "emptyfi");
    }

    dbs = malloc(sizeof(struct disc_block) * sb.num_blocks);
    for (int i = 0; i < sb.num_blocks; i++) {
        dbs[i].next_block_num = -1;
    }
}

void sync_fs() {
    FILE *file;
    file = fopen("fs_data", "w+");

    // superblock
    fwrite(&sb, sizeof(struct superblock), 1, file);

    for (int i = 0; i < sb.num_inodes; i++) {
        fwrite(&inodes[i], sizeof(struct inode), 1, file);
    }

    for (int i = 0; i < sb.num_blocks; i++) {
        fwrite(&dbs[i], sizeof(struct disc_block), 1, file);
    }

    fclose(file);
}

void print_fs() {
    printf("Superblock info\n");
    printf("\tnum inodes %d\n", sb.num_inodes);
    printf("\tnum blocks %d\n", sb.num_blocks);
    printf("\tsize blocks %d\n", sb.size_blocks);

    printf("inodes\n");
    for (int i = 0; i < sb.num_inodes; i++) {
        printf("\tsize: %d block: %d name: %s\n",inodes[i].size, inodes[i].first_block, inodes[i].name);
    }

    for (int i = 0; i < sb.num_blocks; i++) {
        printf("\tblock num: %d next block: %d\n",i, dbs[i].next_block_num);
    }
}

int allocate_file(int size, const char *name) {
    //find an empty inode
    int in = find_empty_inode();
    //find / claim a disk block
    int block = find_empty_block();
    //claim them
    // inode[in].size = size;
    inodes[in].first_block = block;
    dbs[block].next_block_num = -2;
    strcpy(inodes[in].name, name);
    //return the file descriptor
    return in;
}

void set_filesize(int filenum, int size){
    //how many blocks should we have
    int tmp = size + BLOCKSIZE - 1;
    int num = tmp / BLOCKSIZE;
    
    int bn = inodes[filenum].first_block;
    num--;
    //grow the file if necessary
    while (num > 0){
        //check next block number
        int next_num = dbs[bn].next_block_num;
        if (next_num == -2){
            int empty = find_empty_block();
            dbs[bn].next_block_num = empty;
            dbs[empty].next_block_num = -2;
        }
        bn = dbs[bn].next_block_num;
        num--;
    }

    // shorten if necessary
    shorten_file(bn);
    dbs[bn].next_block_num = -2;

}

void write_byte(int filenum, int pos, char * data){
    // calculate which block
    int relative_block = pos / BLOCKSIZE;
    //find the block number
    int bn = get_block_num(filenum, relative_block);
    // calculate the offset in the block
    int offset = pos % BLOCKSIZE;
    //write the data
    dbs[bn].data[offset] = (*data);
}


