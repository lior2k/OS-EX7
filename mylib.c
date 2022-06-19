#include "mylib.h"

struct myFILE *myfopen(const char *pathname, const char *mode) {
    int fd = myopen(pathname, -1);
    if (fd == -1) {
        perror("file not found\n");
        return NULL;
    }
    if (strlen(mode) == 0) {
        perror("mode error\n");
        return NULL;
    }
    struct myFILE *file = (struct myFILE*)malloc(sizeof(struct myFILE));
    file->fd = fd;
    file->pointer = 0;
    if (mode[0] == 'a') mylseek(file->fd, 0, SEEK_END);
    return file;
}
int myfclose(struct myFILE *stream) {
    if (myclose(stream->fd) == 0) {
        free(stream);
        return 1;
    } else {
        free(stream);
        return -1;
    }
}

size_t myfread(void *ptr, size_t size, size_t nmemb, struct myFILE *stream) {
    return myread(stream->fd, ptr, nmemb * size);
}

size_t myfwrite(const void *ptr, size_t size, size_t nmemb, struct myFILE *stream) {
    int pos = mywrite(stream->fd, ptr, size * nmemb);
    sync_fs(stream->name);
    return pos;
}

int myfseek(struct myFILE *stream, long offset, int whence) {
    mylseek(stream->fd, offset, whence);
    return 0;
}

int myfscanf(struct myFILE *stream, const char *format, ...) {
      
}

int myfprintf(struct myFILE *stream, const char *format, ...) {

}