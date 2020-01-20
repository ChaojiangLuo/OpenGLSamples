//
// Created by luocj on 2/12/18.
//
#include <malloc.h>

#include <utils/GLUtils.h>

#include "utils.h"
#include "PngHandler.h"

void GetPNGtextureInfo(int color_type,  gl_texture_t *texinfo) {
    switch (color_type ) {
        case PNG_COLOR_TYPE_GRAY:
        texinfo->format = GL_LUMINANCE;
        texinfo->internalFormat = 1;
        break;

        case PNG_COLOR_TYPE_GRAY_ALPHA:
            texinfo->format = GL_LUMINANCE_ALPHA;
            texinfo->internalFormat = 2;
            break;

        case PNG_COLOR_TYPE_RGB:
            texinfo->format = GL_RGB;
            texinfo->internalFormat = 3;
            break;

        case PNG_COLOR_TYPE_RGB_ALPHA:
            texinfo->format = GL_RGBA;
            texinfo->internalFormat = 4;
            break;

        default:
            /* Badness */
            break;
    }
}

gl_texture_t* readPngFile(char *name) {
    ALOGE("readPngFile %s\n",  name);
    // 前边几句是扯淡，初始化各种结构
    FILE *file = fopen(name, "rb");

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

    png_infop info_ptr = png_create_info_struct(png_ptr);

    setjmp(png_jmpbuf(png_ptr));

    // 这句很重要
    png_init_io(png_ptr, file);

    // 读文件了
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0);

    // 得到文件的宽高色深
    int m_width = png_get_image_width(png_ptr, info_ptr);
    int m_height = png_get_image_height(png_ptr, info_ptr);

    int color_type = png_get_color_type(png_ptr, info_ptr);

    gl_texture_t *texinfo = (gl_texture_t *) malloc(sizeof(gl_texture_t));
    texinfo->width = m_width;
    texinfo->height = m_height;


    GetPNGtextureInfo(color_type, texinfo);

    ALOGE("xm-gfx-opengl: texinfo size %dx%d internalFormat=%d format=%d", texinfo->width, texinfo->height, texinfo->internalFormat, texinfo->format);

    texinfo->texels = (GLubyte *) malloc(sizeof(GLubyte) * texinfo->width * texinfo->height * texinfo->internalFormat);
    /* Setup a pointer array. Each one points at the begening of a row. */
    png_bytep *row_pointers = png_get_rows(png_ptr, info_ptr);

    int pos = 0;
    for (int i = 0; i < m_height; i++) {
        for (int j = 0; j < (texinfo->internalFormat * m_width); j += texinfo->internalFormat) {
            char log[20]={0};
            char logt[5]={0};
            for(int index = 0; index < texinfo->internalFormat; index++) {
                texinfo->texels[pos] = row_pointers[i][j + index];   // r g b a
                sprintf(logt, "%02x ", texinfo->texels[pos]);
                strcat(log, logt);
                pos++;
            }
            ALOGE("xm-gfx-opengl: %s", log);
        }
    }

    // 好了，你可以用这个数据作任何的事情了。。。把它显示出来或者打印出来都行。
    png_destroy_read_struct(&png_ptr, &info_ptr, 0);

    fclose(file);

    return texinfo;
}

void writePngFile(char *fileName, png_byte* src , int width, int height, int format)
{
    if(format != PNG_COLOR_TYPE_RGBA) {
        ALOGE("We just support RGBA format now~");
        return;
    }

    png_structp png_ptr;
    png_infop info_ptr;
    png_colorp palette;

    FILE *fp = fopen(fileName, "wb");
    if (fp == NULL)
        return ;

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL)
    {
        fclose(fp);
        return ;
    }

    /* Allocate/initialize the image information data.  REQUIRED */
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL)
    {
        fclose(fp);
        png_destroy_write_struct(&png_ptr,  NULL);
        return ;
    }
    if (setjmp(png_jmpbuf(png_ptr)))
    {
        /* If we get here, we had a problem writing the file */
        fclose(fp);
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return ;
    }

    /* 接下来告诉 libpng 用 fwrite 来写入 PNG 文件，并传给它已按二进制方式打开的 FILE* fp */
    png_init_io(png_ptr, fp);

    /* 设置png文件的属性 */
    png_set_IHDR(png_ptr, info_ptr, width, height, 8,
                 PNG_COLOR_TYPE_RGBA,
                 PNG_INTERLACE_NONE,
                 PNG_COMPRESSION_TYPE_BASE,
                 PNG_FILTER_TYPE_BASE);

    /* 分配调色板空间。常数 PNG_MAX_PALETTE_LENGTH 的值是256 */
    palette = (png_colorp)png_malloc(png_ptr, PNG_MAX_PALETTE_LENGTH * sizeof(png_color));

    png_set_PLTE(png_ptr, info_ptr, palette, PNG_MAX_PALETTE_LENGTH);

    /* Write the file header information.  REQUIRED */
    png_write_info(png_ptr, info_ptr);

    /* The easiest way to write the image (you may have a different memory
    * layout, however, so choose what fits your needs best).  You need to
    * use the first method if you aren't handling interlacing yourself.
    */
    png_uint_32 k;
    png_byte *image;
    png_bytep row_pointers[height];

    image = src;

    if (height > PNG_UINT_32_MAX/sizeof(png_bytep))
        png_error (png_ptr, "Image is too tall to process in memory");

    for (k = 0; k < height; k++)
        row_pointers[k] = image + k*width*4;

    /* One of the following output methods is REQUIRED */
    png_write_image(png_ptr, row_pointers);

    //end，进行必要的扫尾工作：
    png_write_end(png_ptr, info_ptr);
    png_free(png_ptr, palette);
    png_destroy_write_struct(&png_ptr, &info_ptr);

    fclose(fp);
    ALOGE("success write png file %s\n", fileName);
    return ;
}

void buildPngFile(char *fileName) {
    int width = 4;
    int height = 2;

    png_byte src[] = {
            0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff, 0x00, 0xff,
            0xff, 0xff, 0xff, 0x7f, 0x00, 0xff, 0x00, 0x7f, 0xff, 0x00, 0x00, 0x7f, 0x00, 0x00, 0xff, 0x7f
    };

    writePngFile(fileName,src,width, height, PNG_COLOR_TYPE_RGBA);

    readPngFile(fileName);
}