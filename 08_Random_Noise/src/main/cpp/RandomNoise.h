//
// Created by luocj on 20-4-15.
//

#ifndef OPENGLSAMPLES_RANDOMNOISE_H
#define OPENGLSAMPLES_RANDOMNOISE_H

#include "PngHandler.h"

typedef struct {
    // Handle to a program object
    GLuint programObject;

    // Sampler location
    GLint samplerLoc;

    // Offset location
    GLint offsetLoc;

    GLint resolutionLoc;

    // Texture handle
    GLuint textureId;

    gl_texture_t *texture;
} UserData;

#endif //OPENGLSAMPLES_RANDOMNOISE_H
