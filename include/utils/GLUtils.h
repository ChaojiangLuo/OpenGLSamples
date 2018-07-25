//
// Created by luocj on 18-7-13.
//

#ifndef OPENGLSAMPLES_GL_UTILS_H
#define OPENGLSAMPLES_GL_UTILS_H

#ifndef LOG_TAG
#define LOG_TAG "GL_Error"
#endif

#include <GLES3/gl3.h>
#include <GLES/gl.h>

#include <androidnative/MyLog.h>

static void checkGLErrors() {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
        MyLOGE("GL Error: 0x%04x\n", (int)error);
}

#endif //OPENGLSAMPLES_GL_UTILS_H
