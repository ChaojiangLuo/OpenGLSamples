//
// Created by luocj on 20-4-8.
//

#ifndef OPENGLSAMPLES_PBUFFERDEMO_H
#define OPENGLSAMPLES_PBUFFERDEMO_H

#include "PBufferDraw.h"

typedef struct {
    // Handle to a program object
    GLuint programObject;

    GLuint pProgramObject;

    // Sampler location
    GLint samplerLoc;

    GLint uuColor;

    GLboolean isTexture;

    GLboolean isFragTexture;

    // Offset location
    GLint offsetLoc;

    EGLSurface pSurface;

    gl_texture_t *texture;

    GLint puColor;
} UserData;

#endif //OPENGLSAMPLES_PBUFFERDEMO_H
