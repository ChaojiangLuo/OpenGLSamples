//
// Created by luocj on 2/12/18.
//

#ifndef ANDROIDDEBUGDEMOS_UTILS_H
#define ANDROIDDEBUGDEMOS_UTILS_H

#include <android/log.h>
#include <string.h>

typedef unsigned char byte;

struct engine {
    struct android_app *app;

    int animating;
};

int myHex2Str(const byte *src, byte *dest, int nSrcLen);

void my_show_byte(const char *str, byte bytes[], int size);

#endif //ANDROIDDEBUGDEMOS_UTILS_H
