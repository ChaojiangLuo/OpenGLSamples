//
// Created by luocj on 20-4-8.
//
#undef  LOG_TAG
#define LOG_TAG "PBufferDemo"

#include <androidnative/NativeMain.h>
#include <androidnative/ShaderLoader.h>

#include <utils/GLUtils.h>

#include "PBufferDemo.h"
#include "PBufferDraw.h"

#include "utils.h"

///
// Initialize the shader and program object
//
int Init(Engine *esContext) {
    UserData *userData = (UserData *) esContext->userData;

    char *vShaderSource = readShaderSrcFile("shaders/vertex.vert", esContext->assetManager);
    char *fShaderSource = readShaderSrcFile("shaders/fragment.frag", esContext->assetManager);

    // Load the shaders and get a linked program object
    userData->programObject = loadProgram(vShaderSource, fShaderSource);

    // Get the offset location
    userData->uuColor = glGetUniformLocation(userData->programObject, "uuColor");

    userData->isTexture = glGetUniformLocation(userData->programObject, "isTexture");

    userData->isFragTexture = glGetUniformLocation(userData->programObject, "isFragTexture");

    // Get the sampler location
    userData->samplerLoc = glGetUniformLocation(userData->programObject, "s_texture");

    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

    makeOutputSurface(esContext);

    renderPBuffer(esContext);

    return GL_TRUE;
}

///
// Draw a triangle using the shader pair created in Init()
//
void Draw(Engine *esContext) {
    UserData *userData = (UserData *) esContext->userData;

    //1. Draw trangle
    GLfloat vVertices[] = {-0.5f, 0.5f, 0.0f, 1.0f,
                           -1.0f, -0.5f, 0.0f, 1.0f,
                           0.0f, -0.5f, 0.0f, 1.0f
    };

    ALOGE("xm-gfx:Draw");

    eglMakeCurrent(esContext->display, esContext->surface, esContext->surface, esContext->context);

    // Set the viewport
    glViewport(0, 0, esContext->width, esContext->height);

    glClearColor(1.0f, 0, 1.0f, 1.0f);

    // Clear the color buffer
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // Use the program object
    glUseProgram(userData->programObject);

    glUniform1i(userData->isTexture, false);
    glUniform1i(userData->isFragTexture, false);

    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, vVertices);

    glEnableVertexAttribArray(0);

    glUniform4f(userData->uuColor, 0.0f, 1.0f, 1.0f, 1.0f);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    eglSwapBuffers(esContext->display, esContext->surface);

    // 2. DrawTexture
    GLfloat tVertices[] = {0.0f, 0.5f, 0.0f, 1.0f,   // Position 0
                           -1.0f, -1.0f,              // TexCoord 0
                           0.0f, -0.5f, 0.0f, 1.0f,  // Position 1
                           -1.0f, 2.0f,               // TexCoord 1
                           1.0f, -0.5f, 0.0f, 1.0f,   // Position 2
                           2.0f, 2.0f,                // TexCoord 2
                           1.0f, 0.5f, 0.0f, 1.0f,    // Position 3
                           2.0f, -1.0f                // TexCoord 3
    };
    GLushort indices[] = {0, 1, 2, 0, 2, 3};

    glUniform1i(userData->isTexture, true);

    glUniform1i(userData->isFragTexture, true);

    // Load the vertex position
    glVertexAttribPointer(1, 4, GL_FLOAT,
                          GL_FALSE, 6 * sizeof(GLfloat), tVertices);
    // Load the texture coordinate;
    glVertexAttribPointer(2, 2, GL_FLOAT,
                          GL_FALSE, 6 * sizeof(GLfloat), &tVertices[4]);

    glEnableVertexAttribArray(1);
    glEnableVertexAttribArray(2);

    // Bind the texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, userData->texture->id);

    // Set the sampler texture unit to 0
    glUniform1i(userData->samplerLoc, 0);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

    eglSwapBuffers(esContext->display, esContext->surface);
}

///
// Cleanup
//
void ShutDown(Engine *esContext) {
    UserData *userData = (UserData *) esContext->userData;

    // Delete program object
    glDeleteProgram(userData->programObject);

    termPBuffer(esContext);
}

int appMain(Engine *esContext) {
    esContext->userData = malloc(sizeof(UserData));

    if (!Init(esContext)) {
        return GL_FALSE;
    }

    registerDrawFunc(esContext, Draw);
    registerShutdownFunc(esContext, ShutDown);

    return GL_TRUE;
}