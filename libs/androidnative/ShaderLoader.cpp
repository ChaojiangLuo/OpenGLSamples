//
// Created by luocj on 18-7-16.
//
#define LOG_TAG "ShaderLoader"
#define ATRACE_TAG ATRACE_TAG_GRAPHICS

#include <android/trace.h>

#include <androidnative/ShaderLoader.h>

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