#undef  LOG_TAG
#define LOG_TAG "TextureWrap"

#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include <unistd.h>

#include <android/trace.h>

#include <androidnative/NativeMain.h>
#include <androidnative/ShaderLoader.h>

#include <utils/GLUtils.h>

#include "PngHandler.h"

#include "utils.h"

typedef struct {
    bool inited;
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

    long frame;

} UserData;

char *readShaderSrcFile(const char *shaderFile, AAssetManager *pAssetManager) {
    ATrace_beginSection("xm-gfx:ReadShaderSrcFile");
    AAsset *pAsset = NULL;
    char *pBuffer = NULL;
    off_t size = -1;
    int numByte = -1;

    if (NULL == pAssetManager) {
        MyLOGD("pAssetManager is null!");
        return NULL;
    }
    pAsset = AAssetManager_open(pAssetManager, shaderFile, AASSET_MODE_UNKNOWN);
    //LOGI("after AAssetManager_open");

    size = AAsset_getLength(pAsset);
    MyLOGD("after AAssetManager_open");
    pBuffer = (char *) malloc(size + 1);
    pBuffer[size] = '\0';

    numByte = AAsset_read(pAsset, pBuffer, size);
    MyLOGD("%s : [%s]", shaderFile, pBuffer);
    AAsset_close(pAsset);

    ATrace_endSection();

    return pBuffer;
}


GLuint LoadShader(GLenum type, const char *shaderSrc) {
    ATrace_beginSection("xm-gfx:LoadShader");
    GLuint shader;
    GLint compiled;

    // Create the shader object
    shader = glCreateShader(type);

    if (shader == 0) {
        ATrace_endSection();
        return 0;
    }

    // Load the shader source
    glShaderSource(shader, 1, &shaderSrc, NULL);

    // Compile the shader
    glCompileShader(shader);

    // Check the compile status
    glGetShaderiv(shader, GL_COMPILE_STATUS, &compiled);

    if (!compiled) {
        GLint infoLen = 0;

        glGetShaderiv(shader, GL_INFO_LOG_LENGTH, &infoLen);

        if (infoLen > 1) {
            char *infoLog = (char *) malloc(sizeof(char) * infoLen);

            glGetShaderInfoLog(shader, infoLen, NULL, infoLog);
            MyLOGE("Error compiling shader:[%s]", infoLog);

            free(infoLog);
        }

        glDeleteShader(shader);
        ATrace_endSection();
        return 0;
    }
    ATrace_endSection();

    return shader;
}

GLuint loadProgram(const char *vertShaderSrc, const char *fragShaderSrc) {
    GLuint vertexShader;
    GLuint fragmentShader;
    GLuint programObject;
    GLint linked;

    ATrace_beginSection("xm-gfx:LoadProgram");

    // Load the vertex/fragment shaders
    vertexShader = LoadShader(GL_VERTEX_SHADER, vertShaderSrc);

    if (vertexShader == 0) {
        ATrace_endSection();
        return 0;
    }

    fragmentShader = LoadShader(GL_FRAGMENT_SHADER, fragShaderSrc);

    if (fragmentShader == 0) {
        glDeleteShader(vertexShader);
        ATrace_endSection();
        return 0;
    }

    // Create the program object
    programObject = glCreateProgram();

    if (programObject == 0) {
        ATrace_endSection();
        return 0;
    }

    glAttachShader(programObject, vertexShader);
    glAttachShader(programObject, fragmentShader);

    // Link the program
    ATrace_beginSection("xm-gfx:glLinkProgram");
    glLinkProgram(programObject);
    ATrace_endSection();

    // Check the link status
    glGetProgramiv(programObject, GL_LINK_STATUS, &linked);

    if (!linked) {
        GLint infoLen = 0;

        glGetProgramiv(programObject, GL_INFO_LOG_LENGTH, &infoLen);

        if (infoLen > 1) {
            char *infoLog = (char *) malloc(sizeof(char) * infoLen);

            glGetProgramInfoLog(programObject, infoLen, NULL, infoLog);
            MyLOGE("Error linking program:\n%s\n", infoLog);

            free(infoLog);
        }

        glDeleteProgram(programObject);
        ATrace_endSection();
        return 0;
    }

    // Free up no longer needed shader resources
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    ATrace_endSection();

    return programObject;
}

///
// Create a mipmapped 2D texture image
//
GLuint CreateTexture2D(UserData *userData, AAssetManager* assetManager, char *fileName) {
    // Texture object handle
    GLuint textureId;

    userData->texture = readPngFileFromAssets(assetManager, fileName);

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
    if (userData->inited) {
        return 0;
    }

    MyLOGE("xm-gfx:Init......................................................\n");

    ATrace_beginSection("xm-gfx:Init");

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
    sprintf(fileName, "png/%s", "nezha2.png");

    // Load the texture
    userData->textureId = CreateTexture2D(userData, esContext->assetManager, fileName);

    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);

    userData->inited = true;
    userData->frame = 0;

    ATrace_endSection();
    return GL_TRUE;
}

///
// Draw a triangle using the shader pair created in Init()
//
void Draw(Engine *esContext) {
    UserData *userData = (UserData *) esContext->userData;
    if (!userData->inited) {
        Init(esContext);
    }
    userData->frame++;
    ATrace_setCounter("xm-gfx:FrameNumber", userData->frame);
    ATrace_beginSection("xm-gfx:Draw");
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

    ATrace_endSection();
}

///
// Cleanup
//
void ShutDown(Engine *esContext) {
    UserData *userData = (UserData *) esContext->userData;
    if (!userData->inited) {
        return;
    }
    free(userData->texture->pixels);
    free(userData->texture);

    // Delete texture object
    glDeleteTextures(1, &userData->textureId);

    // Delete program object
    glDeleteProgram(userData->programObject);
}

int appMain(Engine *esContext) {
    esContext->userData = malloc(sizeof(UserData));
    UserData *userData = (UserData *) esContext->userData;
    userData->inited = false;

    registerInitFunc(esContext, Init);
    registerDrawFunc(esContext, Draw);
    registerShutdownFunc(esContext, ShutDown);

    return GL_TRUE;
}
