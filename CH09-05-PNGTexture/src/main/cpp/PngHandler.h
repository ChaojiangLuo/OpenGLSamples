//
// Created by luocj on 2/12/18.
//

#ifndef ANDROIDDEBUGDEMOS_PNGTEST_CPP_H
#define ANDROIDDEBUGDEMOS_PNGTEST_CPP_H

#include <png.h>

class gl_texture_t{
public:
    GLsizei width;
    GLsizei height;
    GLenum format;
    GLint internalFormat;
    GLuint id;
    GLubyte *texels;
};

gl_texture_t* readPngFile(char *name);

void GetPNGtextureInfo(int color_type,  gl_texture_t *texinfo);

void writePngFile(char *fileName, png_byte* src , int width, int height, int format);

void buildPngFile(char *fileName);

#endif //ANDROIDDEBUGDEMOS_PNGTEST_CPP_H
