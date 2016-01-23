//
// Created by joe on 1/20/16.
//

#ifndef IBJUPLOADER_BUFFER_H
#define IBJUPLOADER_BUFFER_H

#include <stdbool.h>
#include <stddef.h>
void *createBuf();
size_t writeBuf(void *, void *, size_t);
void *extractFromBuf(void *);
size_t lenbuf(void *);
void destroyBuf(void *);
void *readOffFile(int id, bool);
#endif //IBJUPLOADER_BUFFER_H
