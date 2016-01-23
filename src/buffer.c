//
// Created by joe on 1/21/16.
//

#include <malloc.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include "buffer.h"

typedef struct {
    void *data;
    size_t size;
} ResizableBuffer;

void *createBuf() {
    ResizableBuffer *ret = malloc(sizeof(ResizableBuffer));
    if (ret == NULL) {
        printf("malloc: out of memory\n");
        exit(EXIT_FAILURE);
        return NULL;
    }
    ret->data = NULL;
    ret->size = 0;
    return ret;
}

void destroyBuf(void *pVoid) {
    free(((ResizableBuffer *) pVoid)->data);
    free(pVoid);
}

size_t lenbuf(void *pVoid) {
    return ((ResizableBuffer *) pVoid)->size;
}

void *extractFromBuf(void *pVoid) {
    return ((ResizableBuffer *) pVoid)->data;
}

void *readOffFile(int id, bool stringbuff) {
    ResizableBuffer *buff = createBuf();
    size_t accumulator = stringbuff ? 1025 : 1024;
    buff->size = stringbuff ? 1 : 0;
    buff->data = malloc(accumulator);
    ssize_t readDat;
    while ((readDat = read(id, &(buff->data[buff->size]), accumulator - buff->size)) > 0) {
        buff->size += readDat;
        if (buff->size == accumulator) {
            size_t reallocsize = stringbuff ? (buff->size - 1) * 2 + 1 : buff->size * 2;
            void *newDat = realloc(buff->data, reallocsize); //Double size
            if (newDat == NULL) {
                printf("out of memory\n");
                return NULL;
            } else {
                buff->data = newDat;
                accumulator = reallocsize;
            }
        }
    }

    if (stringbuff) {
        char *chardat = buff->data;
        chardat[buff->size - 1] = 0;
    }
    return buff;
}

size_t writeBuf(void *pVoid, void *data, size_t t) {
    ResizableBuffer *buff = (ResizableBuffer *) pVoid;

    buff->data = realloc(buff->data, buff->size + t);
    if (buff->data == NULL) {
        printf("out of memory");
        return 0;
    }

    memcpy(&(buff->data[buff->size]), data, t);
    buff->size = buff->size + t;
    return t;
}
