#undef  LOG_TAG
#define LOG_TAG "Simple_TextureCubemap"

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

    // Vertex data
    int numIndices;
    GLfloat *vertices;
    GLfloat *normals;
    GLuint *indices;

} UserData;

///
// Create a simple cubemap with a 1x1 face with a different
// color for each face
GLuint CreateSimpleTextureCubemap() {
    GLuint textureId;
    // Six 1x1 RGB faces
    GLubyte cubePixels[6][3] =
            {
                    // Face 0 - Red
                    255, 0, 0,
                    // Face 1 - Green,
                    0, 255, 0,
                    // Face 2 - Blue
                    0, 0, 255,
                    // Face 3 - Yellow
                    255, 255, 0,
                    // Face 4 - Purple
                    255, 0, 255,
                    // Face 5 - White
                    255, 255, 255
            };

    // Generate a texture object
    glGenTextures(1, &textureId);

    // Bind the texture object
    glBindTexture(GL_TEXTURE_CUBE_MAP, textureId);

    // Load the cube face - Positive X
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_X, 0, GL_RGB, 1, 1, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, &cubePixels[0]);

    // Load the cube face - Negative X
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_X, 0, GL_RGB, 1, 1, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, &cubePixels[1]);

    // Load the cube face - Positive Y
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Y, 0, GL_RGB, 1, 1, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, &cubePixels[2]);

    // Load the cube face - Negative Y
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Y, 0, GL_RGB, 1, 1, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, &cubePixels[3]);

    // Load the cube face - Positive Z
    glTexImage2D(GL_TEXTURE_CUBE_MAP_POSITIVE_Z, 0, GL_RGB, 1, 1, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, &cubePixels[4]);

    // Load the cube face - Negative Z
    glTexImage2D(GL_TEXTURE_CUBE_MAP_NEGATIVE_Z, 0, GL_RGB, 1, 1, 0,
                 GL_RGB, GL_UNSIGNED_BYTE, &cubePixels[5]);

    // Set the filtering mode
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    return textureId;

}


///
// Initialize the shader and program object
//
int Init(Engine *esContext) {
    UserData *userData = (UserData *) esContext->userData;
    char vShaderStr[] =
            "#version 300 es                            \n"
                    "layout(location = 0) in vec4 a_position;   \n"
                    "layout(location = 1) in vec3 a_normal;     \n"
                    "out vec3 v_normal;                         \n"
                    "void main()                                \n"
                    "{                                          \n"
                    "   gl_Position = a_position;               \n"
                    "   v_normal = a_normal;                    \n"
                    "}                                          \n";

    char fShaderStr[] =
            "#version 300 es                                     \n"
                    "precision mediump float;                            \n"
                    "in vec3 v_normal;                                   \n"
                    "layout(location = 0) out vec4 outColor;             \n"
                    "uniform samplerCube s_texture;                      \n"
                    "void main()                                         \n"
                    "{                                                   \n"
                    "   outColor = texture( s_texture, v_normal );       \n"
                    "}                                                   \n";

    // Load the shaders and get a linked program object
    userData->programObject = loadProgram(vShaderStr, fShaderStr);

    // Get the sampler locations
    userData->samplerLoc = glGetUniformLocation(userData->programObject, "s_texture");

    // Load the texture
    userData->textureId = CreateSimpleTextureCubemap();

    // Generate the vertex data
    userData->numIndices = esGenSphere(20, 0.75f, &userData->vertices, &userData->normals,
                                       NULL, &userData->indices);


    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    return GL_TRUE;
}

///
// Draw a triangle using the shader pair created in Init()
//
void Draw(Engine *esContext) {
    UserData *userData = (UserData *) esContext->userData;

    // Set the viewport
    glViewport(0, 0, esContext->width, esContext->height);

    // Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT);


    glCullFace(GL_BACK);
    glEnable(GL_CULL_FACE);

    // Use the program object
    glUseProgram(userData->programObject);

    // Load the vertex position
    glVertexAttribPointer(0, 3, GL_FLOAT,
                          GL_FALSE, 0, userData->vertices);
    // Load the normal
    glVertexAttribPointer(1, 3, GL_FLOAT,
                          GL_FALSE, 0, userData->normals);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    // Bind the texture
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_CUBE_MAP, userData->textureId);

    // Set the sampler texture unit to 0
    glUniform1i(userData->samplerLoc, 0);

    glDrawElements(GL_TRIANGLES, userData->numIndices,
                   GL_UNSIGNED_INT, userData->indices);
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

    free(userData->vertices);
    free(userData->normals);
}


int appMain(Engine *esContext) {
    esContext->userData = malloc(sizeof(UserData));

    if (!Init(esContext)) {
        return GL_FALSE;
    }

    registerDrawFunc(esContext, Draw);
    registerShutdownFunc(esContext, ShutDown);

    return GL_TRUE;
}
