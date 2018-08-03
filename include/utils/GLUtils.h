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

typedef struct {
    GLfloat m[4][4];
} ESMatrix;

static void checkGLErrors() {
    GLenum error = glGetError();
    if (error != GL_NO_ERROR)
        MyLOGE("GL Error: 0x%04x\n", (int) error);
}

int esGenCube(float scale, GLfloat **vertices, GLfloat **normals,
              GLfloat **texCoords, GLuint **indices);

void esFrustum(ESMatrix *result, float left, float right, float bottom, float top, float nearZ,
               float farZ);

void esMatrixLoadIdentity(ESMatrix *result);

void esMatrixMultiply(ESMatrix *result, ESMatrix *srcA, ESMatrix *srcB);

void esPerspective(ESMatrix *result, float fovy, float aspect, float nearZ, float farZ);

void esRotate(ESMatrix *result, GLfloat angle, GLfloat x, GLfloat y, GLfloat z);

void esTranslate(ESMatrix *result, GLfloat tx, GLfloat ty, GLfloat tz);

#endif //OPENGLSAMPLES_GL_UTILS_H
