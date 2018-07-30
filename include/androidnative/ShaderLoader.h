//
// Created by luocj on 18-7-16.
//

#ifndef OPENGLSAMPLES_SHADERLOADER_H
#define OPENGLSAMPLES_SHADERLOADER_H

#include <cstdlib>
#include <cstring>

#include <android/asset_manager_jni.h>

#include <GLES3/gl3.h>

#include "MyLog.h"

char *readShaderSrcFile(const char *shaderFile, AAssetManager *pAssetManager);

GLuint LoadShader(GLenum type, const char *shaderSrc);

GLuint loadProgram(const char *vertShaderSrc, const char *fragShaderSrc);

#endif //OPENGLSAMPLES_SHADERLOADER_H
