//
// Created by luocj on 18-7-16.
//

#ifndef OPENGLSAMPLES_VAOVBODEMO_H
#define OPENGLSAMPLES_VAOVBODEMO_H

#include "NativeMain.h"

/**
 * App saved state data.
 */
struct OpenGLRender {
    int isAlive;
};

void appInit(struct Engine *engine);
void appDeinit(struct Engine *engine);
void appDisplay(struct Engine *engine);

#endif //OPENGLSAMPLES_VAOVBODEMO_H
