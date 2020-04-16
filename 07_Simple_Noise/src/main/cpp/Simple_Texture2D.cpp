#undef  LOG_TAG
#define LOG_TAG "SimpleTexture2D"

#include <androidnative/NativeMain.h>
#include <androidnative/ShaderLoader.h>

#include <utils/GLUtils.h>

typedef struct {
    // Handle to a program object
    GLuint programObject;

    // Sampler location
    GLint samplerLoc[2];

    // Texture handle
    GLuint textureId[2];

} UserData;

//xzd add
GLuint CreateRandText() {
#define RANDW   1080
#define RANDH   2400
#define RAND_SEED 17

    GLuint randTextureId;

    static GLubyte randomTex[RANDW][RANDH] = {};
    srand(time(NULL));
    for (int i = 0; i < RANDW; i++) {
        for (int j = 0; j < RANDH; j++) {
            randomTex[i][j] = rand() % RAND_SEED;
        }
    }

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
    glGenTextures(1, &randTextureId);
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, randTextureId);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_LUMINANCE, RANDW, RANDH, 0, GL_LUMINANCE, GL_UNSIGNED_BYTE,
                 randomTex);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);

    return randTextureId;
}

///
// Create a simple 2x2 texture image with four different colors
//
GLuint CreateSimpleTexture2D() {
    // Texture object handle
    GLuint textureId;

    // 2x2 Image, 3 bytes per pixel (R, G, B)
    GLubyte pixels[4 * 3] = {
            200, 0, 0, // Red
            255, 255, 255, // Green
            0, 150, 0, // Blue
            150, 150, 150  // Yellow
    };

    // Use tightly packed data
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Generate a texture object
    glGenTextures(1, &textureId);

    glActiveTexture(GL_TEXTURE0);
    // Bind the texture object
    glBindTexture(GL_TEXTURE_2D, textureId);

    // Load the texture
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, 2, 2, 0, GL_RGB, GL_UNSIGNED_BYTE, pixels);

    // Set the filtering mode
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return textureId;

}

///
// Initialize the shader and program object
//
int Init(Engine *engine) {
    UserData *userData = (UserData *) engine->userData;

    char *vShaderSource = readShaderSrcFile("shaders/vertex.vert", engine->assetManager);
    char *fShaderSource = readShaderSrcFile("shaders/fragment.frag", engine->assetManager);

    // Load the shaders and get a linked program object
    userData->programObject = loadProgram(vShaderSource, fShaderSource);

    // Get the sampler location
    userData->samplerLoc[0] = glGetUniformLocation(userData->programObject, "s_texture");
    userData->samplerLoc[1] = glGetUniformLocation(userData->programObject, "s_RandTex");

    // Load the texture
    userData->textureId[0] = CreateSimpleTexture2D();
    userData->textureId[1] = CreateRandText();

    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    return GL_TRUE;
}

///
// Draw a triangle using the shader pair created in Init()
//
void Draw(Engine *engine) {
    UserData *userData = (UserData *) engine->userData;
    GLfloat vVertices[] = {-1.0f, 1.0f, 0.0f,  // Position 0
                           0.0f, 0.0f,        // TexCoord 0
                           -1.0f, -1.0f, 0.0f,  // Position 1
                           0.0f, 1.0f,        // TexCoord 1
                           1.0f, -1.0f, 0.0f,  // Position 2
                           1.0f, 1.0f,        // TexCoord 2
                           1.0f, 1.0f, 0.0f,  // Position 3
                           1.0f, 0.0f         // TexCoord 3
    };
    GLushort indices[] = {0, 1, 2, 0, 2, 3};

    // Set the viewport
    glViewport(0, 0, engine->width, engine->height);

    // Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT);

    // Use the program object
    glUseProgram(userData->programObject);

    // Load the vertex position
    glVertexAttribPointer(0, 3, GL_FLOAT,
                          GL_FALSE, 5 * sizeof(GLfloat), vVertices);
    // Load the texture coordinate
    glVertexAttribPointer(1, 2, GL_FLOAT,
                          GL_FALSE, 5 * sizeof(GLfloat), &vVertices[3]);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // Bind the texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, userData->textureId[0]);

    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, userData->textureId[1]);

    // Set the sampler texture unit to 0
    glUniform1i(userData->samplerLoc[0], 0);
    glUniform1i(userData->samplerLoc[1], 1);
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
}

///
// Cleanup
//
void ShutDown(Engine *engine) {
    UserData *userData = (UserData *) engine->userData;

    // Delete texture object
    glDeleteTextures(2, userData->textureId);

    // Delete program object
    glDeleteProgram(userData->programObject);
}


int appMain(Engine *engine) {
    engine->userData = malloc(sizeof(UserData));

    if (!Init(engine)) {
        return GL_FALSE;
    }

    registerDrawFunc(engine, Draw);
    registerShutdownFunc(engine, ShutDown);

    return GL_TRUE;
}