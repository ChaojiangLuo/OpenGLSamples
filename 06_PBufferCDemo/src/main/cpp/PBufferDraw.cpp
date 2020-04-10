//
// Created by luocj on 20-4-8.
//
#include <string>

#include <androidnative/NativeMain.h>
#include <androidnative/ShaderLoader.h>
#include <utils/GLUtils.h>

#include "utils.h"

#include "PBufferDraw.h"
#include "PBufferDemo.h"

bool makeOutputSurface(Engine *esContext) {
    UserData *userData = (UserData *) esContext->userData;

    char *vShaderSourceP = readShaderSrcFile("shaders/vertexp.vert", esContext->assetManager);
    char *fShaderSourceP = readShaderSrcFile("shaders/fragmentp.frag", esContext->assetManager);
    // Load the shaders and get a linked program object
    userData->pProgramObject = loadProgram(vShaderSourceP, fShaderSourceP);

    // Get the sampler location
    userData->puColor = glGetUniformLocation(userData->pProgramObject, "uuColor");

    const int attributes[] = {
            EGL_WIDTH, esContext->width,
            EGL_HEIGHT, esContext->height,
            EGL_NONE};
    // 创建失败时返回EGL14.EGL_NO_SURFACE
    userData->pSurface = eglCreatePbufferSurface(esContext->display, esContext->config, attributes);

    return true;
}

bool checkGlError(const char *op, int lineNumber) {
    bool errorFound = false;
    GLint error = glGetError();
    while (error != GL_NO_ERROR) {
        errorFound = true;
        error = glGetError();
        ALOGE("xm-gfx-checkGlError:after %s() (line # %d) glError (0x%x)\n", op, lineNumber, error);
    }
    return errorFound;
}

/**
 * 渲染到各个eglSurface
 */
void renderPBuffer(Engine *esContext) {
    UserData *userData = (UserData *) esContext->userData;
    ALOGE("xm-gfx:renderPBuffer1");
    // 设置当前的上下文环境和输出缓冲区
    eglMakeCurrent(esContext->display, userData->pSurface, userData->pSurface, esContext->context);
    ALOGE("xm-gfx:renderPBuffer2");
    // 设置视窗大小及位置
    glViewport(0, 0, esContext->width, esContext->height);
    // 绘制
    GLfloat tVertices[] = {0.5f, 0.5f, 0.0f, 1.0f,
                           0.0f, -0.5f, 0.0f, 1.0f,
                           1.0f, -0.5f, 0.0f, 1.0f
    };

    ALOGE("xm-gfx:renderPBuffer3");

    glClearColor(0.0f, 0.0f, 1.0f, 1.0f);

    // Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT);

    // Use the program object
    glUseProgram(userData->pProgramObject);
    ALOGE("xm-gfx:renderPBuffer4");
    glVertexAttribPointer(0, 4, GL_FLOAT, GL_FALSE, 0, tVertices);
    glEnableVertexAttribArray(0);

    glUniform4f(userData->puColor, 0.0f, 1.0f, 0.0f, 1.0f);

    glDrawArrays(GL_TRIANGLES, 0, 3);

    ALOGE("xm-gfx:renderPBuffer5");
    // 交换显存(将surface显存和显示器的显存交换)
    eglSwapBuffers(esContext->display, userData->pSurface);
    ALOGE("xm-gfx:renderPBuffer6");
    unsigned char *pixels = new unsigned char[esContext->width * esContext->height * 4];
    if (!pixels) {
        return;
    }
    memset(pixels, 0x00, sizeof(unsigned char) * esContext->width * esContext->height * 4);

    glReadPixels(0, 0, esContext->width, esContext->height, GL_RGBA, GL_UNSIGNED_BYTE, pixels);
    if (checkGlError(__FUNCTION__, __LINE__)) {
        return;
    }

    ALOGE("xm-gfx:renderPBuffer7");
    gl_texture_t *texinfo = (gl_texture_t *) malloc(sizeof(gl_texture_t));
    texinfo->width = esContext->width;
    texinfo->height = esContext->height;
    ALOGE("xm-gfx:renderPBuffer8");
    texinfo->format = GL_RGBA;
    texinfo->internalFormat = 4;

    ALOGE("xm-gfx:renderPBuffer pixels=[%s]\n", pixels);

    if (0) {
        std::string colorline;
        char s[4];
        for (int j = 0; j < esContext->height; j++) {
            colorline = "color rgba:";
            for (int i = 0; i < esContext->width * 4; i++) {
                if (i > 0 && i % 200 == 0) {
                    ALOGE("xm-gfx:renderPBuffer[%dx%d]=[%s]\n", i, j, colorline.c_str());
                    colorline = "color rgba:";
                }
                sprintf(s, "%x ", pixels[j * esContext->width * 4 + i]);
                colorline.append(s);
            }
        }
    }

    texinfo->pixels = pixels;
    ALOGE("xm-gfx:renderPBuffer9");
    // Texture object handle
    GLuint textureId;

    // Generate a texture object
    glGenTextures(1, &textureId);
    ALOGE("xm-gfx:renderPBuffer10");
    // Bind the texture object
    glBindTexture(GL_TEXTURE_2D, textureId);

    texinfo->id = textureId;
    ALOGE("xm-gfx:renderPBuffer11");
    userData->texture = texinfo;

    // Load mipmap level 0
    glTexImage2D(GL_TEXTURE_2D, 0, userData->texture->format, userData->texture->width,
                 userData->texture->height,
                 0, userData->texture->format, GL_UNSIGNED_BYTE, userData->texture->pixels);
    ALOGE("xm-gfx:renderPBuffer12");
    // Set the filtering mode
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    ALOGE("xm-gfx:renderPBuffer13");
}

void termPBuffer(Engine *esContext) {
    UserData *userData = (UserData *) esContext->userData;

    glDeleteProgram(userData->pProgramObject);

    userData->pSurface = EGL_NO_SURFACE;

    free(userData->texture->pixels);
    free(userData->texture);

    // Delete texture object
    glDeleteTextures(1, &userData->texture->id);
}