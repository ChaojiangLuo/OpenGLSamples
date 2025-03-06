//
// Created by luocj on 2/12/18.
//
#include <malloc.h>
#include <vector>

#include <utils/GLUtils.h>
#include <androidnative/NativeMain.h>

#include "utils.h"
#include "PngHandler.h"

void GetPNGtextureInfo(int color_type, gl_texture_t *texinfo) {
    switch (color_type) {
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

gl_texture_t *readPngFile(char *name) {
    ALOGE("readPngFile %s\n", name);
    // 前边几句是扯淡，初始化各种结构
    FILE *file = fopen(name, "rb");
    if (file == nullptr) {
        ALOGE("readPngFile open error %s\n", name);
        return nullptr;
    }
    ALOGE("readPngFile open done %s\n", name);

    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, 0, 0, 0);

    ALOGE("readPngFile png_create_read_struct done %s\n", name);

    png_infop info_ptr = png_create_info_struct(png_ptr);

    setjmp(png_jmpbuf(png_ptr));

    // 这句很重要
    png_init_io(png_ptr, file);

    ALOGE("readPngFile png_init_io done %s\n", name);

    // 读文件了
    png_read_png(png_ptr, info_ptr, PNG_TRANSFORM_EXPAND, 0);

    ALOGE("readPngFile png_read_png done %s\n", name);

    // 得到文件的宽高色深
    int m_width = png_get_image_width(png_ptr, info_ptr);
    int m_height = png_get_image_height(png_ptr, info_ptr);

    int color_type = png_get_color_type(png_ptr, info_ptr);

    ALOGE("readPngFile png_get_color_type done %s\n", name);

    gl_texture_t *texinfo = (gl_texture_t *) malloc(sizeof(gl_texture_t));
    texinfo->width = m_width;
    texinfo->height = m_height;


    GetPNGtextureInfo(color_type, texinfo);

    ALOGE("xm-gfx-opengl: texinfo size %dx%d internalFormat=%d format=%d", texinfo->width,
          texinfo->height, texinfo->internalFormat, texinfo->format);

    texinfo->pixels = (GLubyte *) malloc(
            sizeof(GLubyte) * texinfo->width * texinfo->height * texinfo->internalFormat);
    /* Setup a pointer array. Each one points at the begening of a row. */
    png_bytep *row_pointers = png_get_rows(png_ptr, info_ptr);

    int pos = 0;
    for (int i = 0; i < m_height; i++) {
        for (int j = 0; j < (texinfo->internalFormat * m_width); j += texinfo->internalFormat) {
            char log[20] = {0};
            char logt[5] = {0};
            for (int index = 0; index < texinfo->internalFormat; index++) {
                texinfo->pixels[pos] = row_pointers[i][j + index];   // r g b a
                sprintf(logt, "%02x ", texinfo->pixels[pos]);
                strcat(log, logt);
                pos++;
            }
            //ALOGE("xm-gfx-opengl: %s", log);
        }
    }

    // 好了，你可以用这个数据作任何的事情了。。。把它显示出来或者打印出来都行。
    png_destroy_read_struct(&png_ptr, &info_ptr, 0);

    fclose(file);

    return texinfo;
}

void writePngFile(char *fileName, png_byte *src, int width, int height, int format) {
    if (format != PNG_COLOR_TYPE_RGBA) {
        ALOGE("We just support RGBA format now~");
        return;
    }

    png_structp png_ptr;
    png_infop info_ptr;
    png_colorp palette;

    FILE *fp = fopen(fileName, "wb");
    if (fp == NULL)
        return;

    png_ptr = png_create_write_struct(PNG_LIBPNG_VER_STRING, NULL, NULL, NULL);
    if (png_ptr == NULL) {
        fclose(fp);
        return;
    }

    /* Allocate/initialize the image information data.  REQUIRED */
    info_ptr = png_create_info_struct(png_ptr);
    if (info_ptr == NULL) {
        fclose(fp);
        png_destroy_write_struct(&png_ptr, NULL);
        return;
    }
    if (setjmp(png_jmpbuf(png_ptr))) {
        /* If we get here, we had a problem writing the file */
        fclose(fp);
        png_destroy_write_struct(&png_ptr, &info_ptr);
        return;
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
    palette = (png_colorp) png_malloc(png_ptr, PNG_MAX_PALETTE_LENGTH * sizeof(png_color));

    png_set_PLTE(png_ptr, info_ptr, palette, PNG_MAX_PALETTE_LENGTH);

    /* Write the file header information.  REQUIRED */
    png_write_info(png_ptr, info_ptr);

    /* The easiest way to write the image (you may have a different memory
    * layout, however, so choose what fits your needs best).  You need to
    * use the first method if you aren't handling interlacing yourself.
    */
    png_uint_32 k;
    png_byte *image;
    std::vector<png_bytep> row_pointers(height);

    image = src;

    if (height > PNG_UINT_32_MAX / sizeof(png_bytep))
        png_error(png_ptr, "Image is too tall to process in memory");

    for (k = 0; k < height; k++)
        row_pointers[k] = image + k * width * 4;

    /* One of the following output methods is REQUIRED */
    png_write_image(png_ptr, row_pointers.data());

    //end，进行必要的扫尾工作：
    png_write_end(png_ptr, info_ptr);
    png_free(png_ptr, palette);
    png_destroy_write_struct(&png_ptr, &info_ptr);

    fclose(fp);
    ALOGE("success write png file %s\n", fileName);
    return;
}

void buildPngFile(char *fileName) {
    int width = 4;
    int height = 2;

    png_byte src[] = {
            0x00, 0x00, 0x00, 0xff, 0xff, 0x00, 0x00, 0xff, 0x00, 0x00, 0xff, 0xff, 0x00, 0xff,
            0x00, 0xff,
            0xff, 0xff, 0xff, 0x7f, 0x00, 0xff, 0x00, 0x7f, 0xff, 0x00, 0x00, 0x7f, 0x00, 0x00,
            0xff, 0x7f
    };

    writePngFile(fileName, src, width, height, PNG_COLOR_TYPE_RGBA);

    readPngFile(fileName);
}

// 自定义错误处理函数
void pngErrorHandler(png_structp png_ptr, png_const_charp error_msg) {
    char* name = static_cast<char*>(png_get_error_ptr(png_ptr));
    ALOGE("Error during PNG decoding %s: %s\n", name, error_msg);
    longjmp(png_jmpbuf(png_ptr), 1);
}

// 自定义警告处理函数
void pngWarningHandler(png_structp png_ptr, png_const_charp warning_msg) {
    char* name = static_cast<char*>(png_get_error_ptr(png_ptr));
    ALOGE("Warning during PNG decoding %s: %s\n", name, warning_msg);
}

gl_texture_t *readPngFileFromAssets(AAssetManager *pAssetManager, char *name) {
    if (pAssetManager == nullptr) {
        ALOGE("pAssetManager null %s\n", name);
        return nullptr;
    }

    // 打开 PNG 文件
    AAsset* asset = AAssetManager_open(pAssetManager, name, AASSET_MODE_STREAMING);
    if (asset == nullptr) {
        ALOGE("pAssetManager null, can't open %s\n", name);
        return nullptr;
    }

    // 获取文件大小
    off_t fileSize = AAsset_getLength(asset);
    if (fileSize <= 0) {
        ALOGE("Invalid file size %s\n", name);
        AAsset_close(asset);
        return nullptr;
    }

    // 读取文件数据
    char* buffer = new (std::nothrow) char[fileSize];
    if (buffer == nullptr) {
        ALOGE("Failed to allocate memory for buffer %s\n", name);
        AAsset_close(asset);
        return nullptr;
    }
    int bytesRead = AAsset_read(asset, buffer, fileSize);
    if (bytesRead != fileSize) {
        ALOGE("Failed to read the entire file %s. Read %d bytes, expected %ld bytes\n", name, bytesRead, fileSize);
        delete[] buffer;
        AAsset_close(asset);
        return nullptr;
    }

    // 关闭文件
    AAsset_close(asset);

    // 使用 libpng 解码 PNG 数据
    png_structp png_ptr = png_create_read_struct(PNG_LIBPNG_VER_STRING, name, pngErrorHandler, pngWarningHandler);
    if (!png_ptr) {
        ALOGE("Failed to create PNG read struct %s\n", name);
        delete[] buffer;
        return nullptr;
    }

    png_infop info_ptr = png_create_info_struct(png_ptr);
    if (!info_ptr) {
        ALOGE("Failed to create PNG info struct %s\n", name);
        png_destroy_read_struct(&png_ptr, nullptr, nullptr);
        delete[] buffer;
        return nullptr;
    }

    if (setjmp(png_jmpbuf(png_ptr))) {
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        delete[] buffer;
        return nullptr;
    }

    // 初始化 PNG 读取
    char* bufferPtr = buffer;
    png_set_read_fn(png_ptr, &bufferPtr, [](png_structp png_ptr, png_bytep outBytes, png_size_t byteCountToRead) {
        char** bufferPtr = static_cast<char**>(png_get_io_ptr(png_ptr));
        memcpy(outBytes, *bufferPtr, byteCountToRead);
        *bufferPtr += byteCountToRead;
    });

    // 读取 PNG 头信息
    png_read_info(png_ptr, info_ptr);

    // 获取图像信息
    int width = png_get_image_width(png_ptr, info_ptr);
    int height = png_get_image_height(png_ptr, info_ptr);
    int color_type = png_get_color_type(png_ptr, info_ptr);
    int bit_depth = png_get_bit_depth(png_ptr, info_ptr);

    ALOGE("PNG Image: Width=%d Height=%d %s\n", width, height, name);

    // 处理颜色类型和位深度
    if (bit_depth == 16)
        png_set_strip_16(png_ptr);

    if (color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_palette_to_rgb(png_ptr);

    if (color_type == PNG_COLOR_TYPE_GRAY && bit_depth < 8)
        png_set_expand_gray_1_2_4_to_8(png_ptr);

    if (png_get_valid(png_ptr, info_ptr, PNG_INFO_tRNS))
        png_set_tRNS_to_alpha(png_ptr);

    if (color_type == PNG_COLOR_TYPE_RGB ||
        color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_PALETTE)
        png_set_filler(png_ptr, 0xFF, PNG_FILLER_AFTER);

    if (color_type == PNG_COLOR_TYPE_GRAY ||
        color_type == PNG_COLOR_TYPE_GRAY_ALPHA)
        png_set_gray_to_rgb(png_ptr);

    // 更新图像信息
    png_read_update_info(png_ptr, info_ptr);

    gl_texture_t *texinfo = (gl_texture_t *) malloc(sizeof(gl_texture_t));
    if (texinfo == nullptr) {
        ALOGE("Failed to allocate memory for texinfo %s\n", name);
        delete[] buffer;
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        return nullptr;
    }
    texinfo->width = width;
    texinfo->height = height;

    GetPNGtextureInfo(color_type, texinfo);

    ALOGE("xm-gfx-opengl: texinfo size %dx%d internalFormat=%d format=%d", texinfo->width,
          texinfo->height, texinfo->internalFormat, texinfo->format);

    texinfo->pixels = (GLubyte *) malloc(
            sizeof(GLubyte) * texinfo->width * texinfo->height * texinfo->internalFormat);
    if (texinfo->pixels == nullptr) {
        ALOGE("Failed to allocate memory for pixels %s\n", name);
        free(texinfo);
        delete[] buffer;
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        return nullptr;
    }

    /* Setup a pointer array. Each one points at the beginning of a row. */
    png_bytep *row_pointers = new (std::nothrow) png_bytep[height];
    if (row_pointers == nullptr) {
        ALOGE("Failed to allocate memory for row_pointers %s\n", name);
        free(texinfo->pixels);
        free(texinfo);
        delete[] buffer;
        png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
        return nullptr;
    }
    for (int y = 0; y < height; y++) {
        row_pointers[y] = new (std::nothrow) png_byte[png_get_rowbytes(png_ptr, info_ptr)];
        if (row_pointers[y] == nullptr) {
            ALOGE("Failed to allocate memory for row %d of row_pointers %s\n", y, name);
            for (int i = 0; i < y; i++) {
                delete[] row_pointers[i];
            }
            delete[] row_pointers;
            free(texinfo->pixels);
            free(texinfo);
            delete[] buffer;
            png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);
            return nullptr;
        }
    }

    // 读取图像数据
    png_read_image(png_ptr, row_pointers);

    int pos = 0;
    for (int i = 0; i < height; i++) {
        for (int j = 0; j < (texinfo->internalFormat * width); j += texinfo->internalFormat) {
            for (int index = 0; index < texinfo->internalFormat; index++) {
                texinfo->pixels[pos] = row_pointers[i][j + index];   // r g b a
                pos++;
            }
        }
    }

    // 清理资源
    for (int y = 0; y < height; y++) {
        delete[] row_pointers[y];
    }
    delete[] row_pointers;
    delete[] buffer;
    png_destroy_read_struct(&png_ptr, &info_ptr, nullptr);

    return texinfo;
}