//
// Created by luocj on 18-7-16.
//
#define LOG_TAG "ShaderLoader"

#include "ShaderLoader.h"

char *readShaderSrcFile(const char *shaderFile, AAssetManager *pAssetManager) {
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

    return pBuffer;
}


GLuint LoadShader(GLenum type, const char *shaderSrc) {
    GLuint shader;
    GLint compiled;

    // Create the shader object
    shader = glCreateShader(type);

    if (shader == 0) {
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
        return 0;
    }

    return shader;
}