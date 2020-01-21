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

    free(userData->texture->pixels);
    free(userData->texture);

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
    GLfloat vVertices[] = {-0.3f, 0.3f, 0.0f, 1.0f,  // Position 0
                           -1.0f, -1.0f,              // TexCoord 0
                           -0.3f, -0.3f, 0.0f, 1.0f, // Position 1
                           -1.0f, 2.0f,              // TexCoord 1
                           0.3f, -0.3f, 0.0f, 1.0f, // Position 2
                           2.0f, 2.0f,              // TexCoord 2
                           0.3f, 0.3f, 0.0f, 1.0f,  // Position 3
                           2.0f, -1.0f               // TexCoord 3
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

    // Draw quad with repeat wrap mode
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glUniform1f(userData->offsetLoc, -0.7f);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

    // Draw quad with clamp to edge wrap mode
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glUniform1f(userData->offsetLoc, 0.0f);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);

    // Draw quad with mirrored repeat
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_MIRRORED_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_MIRRORED_REPEAT);
    glUniform1f(userData->offsetLoc, 0.7f);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
}

///
// Cleanup
//
void ShutDown(Engine *esContext) {
    UserData *userData = (UserData *) esContext->userData;

    // Delete texture object
    glDeleteTextures(1, &userData->textureId);

    // Delete program object
    glDeleteProgram(userData->programObject);
}

#ifdef __SIZEOF_LONG__
#define BITS_PER_LONG (__CHAR_BIT__ * __SIZEOF_LONG__)
#else
#define BITS_PER_LONG __WORDSIZE
#endif

#define BIT_MASK(nr)        (1UL << ((nr) % BITS_PER_LONG))
#define BIT_WORD(nr)        ((nr) / BITS_PER_LONG)

static __always_inline int test_bit(unsigned int nr, const unsigned long *addr) {
    return ((1UL << (nr % __BITS_PER_LONG)) &
            (((unsigned long *) addr)[nr / __BITS_PER_LONG])) != 0;
}

static inline void __change_bit(int nr, volatile void *addr) {
    unsigned long mask = BIT_MASK(nr);
    unsigned long *p = ((unsigned long *) addr) + BIT_WORD(nr);

    *p ^= mask;
}

#define KEY_MAX            0x2ff
#define KEY_CNT            (KEY_MAX+1)

#define __KERNEL_DIV_ROUND_UP(n, d) (((n) + (d) - 1) / (d))
#define DIV_ROUND_UP __KERNEL_DIV_ROUND_UP
#define BITS_PER_BYTE        8
#define BITS_TO_LONGS(nr)    DIV_ROUND_UP(nr, BITS_PER_BYTE * sizeof(long))

struct input_dev {
    unsigned long key[BITS_TO_LONGS(KEY_CNT)];
};

int appMain(Engine *esContext) {
    esContext->userData = malloc(sizeof(UserData));

    if (!Init(esContext)) {
        return GL_FALSE;
    }

    registerDrawFunc(esContext, Draw);
    registerShutdownFunc(esContext, ShutDown);

    bool debug = true;

    if (debug) {
        ALOGE("xm-gfx:appMain debug=%d\n", debug);
        if (debug) {
        }
    }

    uint32_t z = 12;

    uint32_t fod = (z & 0xFF000000) >> 24;
    z &= 0x00FFFFFF;

    ALOGE("xm-gfx:appMain fod=%d z=%d\n", fod, z);

    unsigned int code = 338;

    input_dev *dev = new input_dev;

    int value = 1;

    memset(dev->key, 0, sizeof(dev->key));

    int disposition = 0;

    ((unsigned long *) dev->key)[code / __BITS_PER_LONG] = 262144;

    if (!!test_bit(code, dev->key) != !!value) {
        ALOGE("xm-gfx:appMain debug=%d value=%lu\n", disposition,
              (((unsigned long *) dev->key)[code / __BITS_PER_LONG]));
        __change_bit(code, dev->key);
        disposition = 1;
    }

    ALOGE("xm-gfx:appMain disposition=%d value=%lu size=%zu\n", disposition,
          (((unsigned long *) dev->key)[code / __BITS_PER_LONG]), sizeof(dev->key));

    return GL_TRUE;
}
