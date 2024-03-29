#undef  LOG_TAG
#define LOG_TAG "TextureWrap"

#include <androidnative/NativeMain.h>
#include <androidnative/ShaderLoader.h>

#include <utils/GLUtils.h>

#include "PngHandler.h"

#include "utils.h"

typedef struct {
    // Handle to a program object
    GLuint programObject;

    // Sampler location
    GLint samplerLoc;

    // Offset location
    GLint offsetLoc;

    // Texture handle
    GLuint textureId;

    GLboolean blur;

    float texelWidthOffset;
    float texelHeightOffset;

    gl_texture_t *texture;

} UserData;


///
// Create a mipmapped 2D texture image
//
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

    // Get the sampler location
    userData->samplerLoc = glGetUniformLocation(userData->programObject, "s_texture");

    // Get the offset location
    userData->offsetLoc = glGetUniformLocation(userData->programObject, "u_offset");

    userData->blur = glGetUniformLocation(userData->programObject, "blur");

    userData->texelWidthOffset = glGetUniformLocation(userData->programObject, "texelWidthOffset");

    userData->texelHeightOffset = glGetUniformLocation(userData->programObject,
                                                       "texelHeightOffset");

    char fileName[512] = {0};
    sprintf(fileName, "/sdcard/png/%s", "png_4_2_32bit.png");

    // Load the texture
    userData->textureId = CreateTexture2D(userData, fileName);

    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    return GL_TRUE;
}

///
// Draw a triangle using the shader pair created in Init()
//
void Draw(Engine *esContext) {
    UserData *userData = (UserData *) esContext->userData;
    GLfloat vVertices[] = {-0.3f, 0.3f, 0.0f, 1.0f,   // Position 0
                           -1.0f, -1.0f,              // TexCoord 0
                           -0.3f, -0.3f, 0.0f, 1.0f,  // Position 1
                           -1.0f, 2.0f,               // TexCoord 1
                           0.3f, -0.3f, 0.0f, 1.0f,   // Position 2
                           2.0f, 2.0f,                // TexCoord 2
                           0.3f, 0.3f, 0.0f, 1.0f,    // Position 3
                           2.0f, -1.0f                // TexCoord 3
    };
    GLushort indices[] = {0, 1, 2, 0, 2, 3};

    // Set the viewport
    glViewport(0, 0, esContext->width, esContext->height);

    // Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT);

    // Use the program object
    glUseProgram(userData->programObject);

    // Load the vertex position
    glVertexAttribPointer(0, 4, GL_FLOAT,
                          GL_FALSE, 6 * sizeof(GLfloat), vVertices);
    // Load the texture coordinate
    glVertexAttribPointer(1, 2, GL_FLOAT,
                          GL_FALSE, 6 * sizeof(GLfloat), &vVertices[4]);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // Bind the texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, userData->textureId);

    // Set the sampler texture unit to 0
    glUniform1i(userData->samplerLoc, 0);

    glUniform1i(userData->blur, 0);
    // Draw quad with clamp to border wrap mode
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    glUniform1f(userData->offsetLoc, -0.75f);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

    glUniform1i(userData->blur, 0);
    // Draw quad with repeat wrap mode
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glUniform1f(userData->offsetLoc, -0.25f);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

    glUniform1i(userData->blur, 1);
    glUniform1f(userData->texelWidthOffset, 1.0f / userData->texture->width);
    glUniform1f(userData->texelHeightOffset, 1.0f / userData->texture->height);
    // Draw quad with clamp to edge wrap mode
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glUniform1f(userData->offsetLoc, 0.25f);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

    glUniform1i(userData->blur, 0);
    // Draw quad with mirrored repeat
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glUniform1f(userData->offsetLoc, 0.75f);
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
