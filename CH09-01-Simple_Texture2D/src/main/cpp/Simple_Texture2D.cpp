#undef  LOG_TAG
#define LOG_TAG "SimpleTexture2D"

#include <androidnative/NativeMain.h>
#include <androidnative/ShaderLoader.h>

#include <utils/GLUtils.h>

typedef struct {
    // Handle to a program object
    GLuint programObject;

    // Sampler location
    GLint samplerLoc;

    // Texture handle
    GLuint textureId;

} UserData;

///
// Create a simple 2x2 texture image with four different colors
//
GLuint CreateSimpleTexture2D() {
    // Texture object handle
    GLuint textureId;

    // 2x2 Image, 3 bytes per pixel (R, G, B)
    GLubyte pixels[4 * 3] =
            {
                    255, 0, 0, // Red
                    0, 255, 0, // Green
                    0, 0, 255, // Blue
                    255, 255, 0  // Yellow
            };

    // Use tightly packed data
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

    // Generate a texture object
    glGenTextures(1, &textureId);

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
    char vShaderStr[] =
            "#version 300 es                            \n"
                    "layout(location = 0) in vec4 a_position;   \n"
                    "layout(location = 1) in vec2 a_texCoord;   \n"
                    "out vec2 v_texCoord;                       \n"
                    "void main()                                \n"
                    "{                                          \n"
                    "   gl_Position = a_position;               \n"
                    "   v_texCoord = a_texCoord;                \n"
                    "}                                          \n";

    char fShaderStr[] =
            "#version 300 es                                     \n"
                    "precision mediump float;                            \n"
                    "in vec2 v_texCoord;                                 \n"
                    "layout(location = 0) out vec4 outColor;             \n"
                    "uniform sampler2D s_texture;                        \n"
                    "void main()                                         \n"
                    "{                                                   \n"
                    "  outColor = texture( s_texture, v_texCoord );      \n"
                    "}                                                   \n";

    // Load the shaders and get a linked program object
    userData->programObject = loadProgram(vShaderStr, fShaderStr);

    // Get the sampler location
    userData->samplerLoc = glGetUniformLocation(userData->programObject, "s_texture");

    // Load the texture
    userData->textureId = CreateSimpleTexture2D();

    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    return GL_TRUE;
}

///
// Draw a triangle using the shader pair created in Init()
//
void Draw(Engine *engine) {
    UserData *userData = (UserData *) engine->userData;
    GLfloat vVertices[] = {-0.5f, 0.5f, 0.0f,  // Position 0
                           0.0f, 0.0f,        // TexCoord 0
                           -0.5f, -0.5f, 0.0f,  // Position 1
                           0.0f, 1.0f,        // TexCoord 1
                           0.5f, -0.5f, 0.0f,  // Position 2
                           1.0f, 1.0f,        // TexCoord 2
                           0.5f, 0.5f, 0.0f,  // Position 3
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
    glBindTexture(GL_TEXTURE_2D, userData->textureId);

    // Set the sampler texture unit to 0
    glUniform1i(userData->samplerLoc, 0);

    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
}

///
// Cleanup
//
void ShutDown(Engine *engine) {
    UserData *userData = (UserData *) engine->userData;

    // Delete texture object
    glDeleteTextures(1, &userData->textureId);

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
