//
// Created by luocj on 20-4-15.
//
#undef  LOG_TAG
#define LOG_TAG "RandomNoise"

#include <androidnative/NativeMain.h>
#include <androidnative/ShaderLoader.h>
#include <utils/GLUtils.h>

#include "RandomNoise.h"

GLuint CreateTexture2D(UserData *userData, char *fileName) {
    // Texture object handle
    GLuint textureId;

    userData->texture = readPngFile(fileName);

    // Generate a texture object
    glGenTextures(1, &textureId);

    // Bind the texture object
    glBindTexture(GL_TEXTURE_2D, textureId);

    // Load mipmap level 0
    glTexImage2D(GL_TEXTURE_2D, 0, userData->texture->format, userData->texture->width,
                 userData->texture->height,
                 0, userData->texture->format, GL_UNSIGNED_BYTE, userData->texture->pixels);

    // Set the filtering mode
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    return textureId;
}

///
// Initialize the shader and program object
//
int Init(Engine *esContext) {
    UserData *userData = (UserData *) esContext->userData;

    char *vShaderSource = readShaderSrcFile("shaders/vertex.vert", esContext->assetManager);
    char *fShaderSource = readShaderSrcFile("shaders/fragment.frag", esContext->assetManager);

    // Load the shaders and get a linked program object
    userData->programObject = loadProgram(vShaderSource, fShaderSource);

    userData->offsetLoc = glGetUniformLocation(userData->programObject, "v_offset");

    userData->resolutionLoc = glGetUniformLocation(userData->programObject, "u_resolution");

    // Get the sampler location
    userData->samplerLoc = glGetUniformLocation(userData->programObject, "s_texture");

    char fileName[512] = {0};
    sprintf(fileName, "/sdcard/png/%s", "png_4_2_32bit.png");

    // Load the texture
    userData->textureId = CreateTexture2D(userData, fileName);

    return GL_TRUE;
}

///
// Draw a triangle using the shader pair created in Init()
//
void Draw(Engine *esContext) {
    UserData *userData = (UserData *) esContext->userData;

    //1. Draw trangle
    GLfloat vertices[] = {-1.0f, 1.0f, 0.0f, 1.0f,
                          -1.0f, -1.0f,              // TexCoord 0
                          -1.0f, -1.0f, 0.0f, 1.0f,
                          -1.0f, 2.0f,               // TexCoord 1
                          1.0f, -1.0f, 0.0f, 1.0f,
                          2.0f, 2.0f,                // TexCoord 2
                          1.0f, 1.0f, 0.0f, 1.0f,
                          2.0f, -1.0f                // TexCoord 3
    };

    GLushort indices[] = {0, 1, 2, 0, 2, 3};

    // Set the viewport
    glViewport(0, 0, esContext->width, esContext->height);

    glClearColor(1.0f, 1.0f, 1.0f, 1.0f);
    // Clear the color buffer
    glClear(GL_DEPTH_BUFFER_BIT | GL_COLOR_BUFFER_BIT);

    // Use the program object
    glUseProgram(userData->programObject);

    // Load the vertex position
    glVertexAttribPointer(0, 4, GL_FLOAT,
                          GL_FALSE, 6 * sizeof(GLfloat), vertices);
    // Load the texture coordinate
    glVertexAttribPointer(1, 2, GL_FLOAT,
                          GL_FALSE, 6 * sizeof(GLfloat), &vertices[4]);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    glUniform2f(userData->offsetLoc, -1.0f, 1.0f);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

    glUniform2f(userData->offsetLoc, -1.0f, -1.0f);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

    glUniform2f(userData->offsetLoc, 1.0f, -1.0f);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

    glUniform2f(userData->offsetLoc, 1.0f, 1.0f);
    glUniform2f(userData->resolutionLoc, esContext->width, esContext->height);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

    // Set the sampler texture unit to 0
    glUniform1i(userData->samplerLoc, 0);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);
    // Bind the texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, userData->textureId);

    // Draw quad with clamp to border wrap mode
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
}

///
// Cleanup
//
void ShutDown(Engine *esContext) {
    UserData *userData = (UserData *) esContext->userData;

    free(userData->texture->pixels);
    free(userData->texture);

    // Delete texture object
    glDeleteTextures(1, &userData->textureId);

    // Delete program object
    glDeleteProgram(userData->programObject);
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