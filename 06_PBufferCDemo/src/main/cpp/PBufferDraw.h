//
// Created by luocj on 20-4-8.
//

#include <EGL/egl.h>

#ifndef OPENGLSAMPLES_PBUFFERDRAW_H
#define OPENGLSAMPLES_PBUFFERDRAW_H

struct gl_texture_t {
    GLsizei width;
    GLsizei height;
    GLenum format;
    GLint internalFormat;
    GLuint id;
    GLubyte *pixels;
};

bool makeOutputSurface(Engine *esContext);

void renderPBuffer(Engine *esContext);

void termPBuffer(Engine *esContext);

#endif //OPENGLSAMPLES_PBUFFERDRAW_H
