//
// Created by luocj on 18-7-16.
//

#ifndef OPENGLSAMPLES_SHADERLOADER_H
#define OPENGLSAMPLES_SHADERLOADER_H

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

char* readShaderFile(const char* path);
int writeShaderFile(char* path, char* source);
unsigned char* readDataFromFile(char* path);

#endif //OPENGLSAMPLES_SHADERLOADER_H
