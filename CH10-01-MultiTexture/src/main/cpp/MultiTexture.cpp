#undef  LOG_TAG
#define LOG_TAG "MultiTexture"

#include <androidnative/NativeMain.h>
#include <androidnative/ShaderLoader.h>

#include <utils/AssetUtils.h>
#include <utils/GLUtils.h>

typedef struct {
    // Handle to a program object
    GLuint programObject;

    // Uniform locations
    GLint mvpLoc;

    // Sampler locations
    GLint baseMapLoc;
    GLint lightMapLoc;

    // Texture handle
    GLuint baseMapTexId;
    GLuint lightMapTexId;

    // MVP matrix
    ESMatrix mvpMatrix;
    GLfloat light;
} UserData;

///
// Load texture from disk
//
GLuint LoadTexture(void *ioContext, const char *fileName) {
    int width, height;

    char *buffer = esLoadTGA(ioContext, fileName, &width, &height);
    GLuint texId;

    if (buffer == NULL) {
        MyLOGE("Error loading (%s) image.\n", fileName);
        return 0;
    }

    glGenTextures(1, &texId);
    glBindTexture(GL_TEXTURE_2D, texId);

    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_RGB, GL_UNSIGNED_BYTE, buffer);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    free(buffer);

    return texId;
}


///
// Initialize the shader and program object
//
int Init(Engine *esContext) {
    UserData *userData = (UserData *) esContext->userData;
    char vShaderStr[] =
            "#version 300 es                            \n"
                    "uniform mat4 u_mvpMatrix;                  \n"
                    "layout(location = 0) in vec4 a_position;   \n"
                    "layout(location = 1) in vec2 a_texCoord;   \n"
                    "out vec2 v_texCoord;                       \n"
                    "void main()                                \n"
                    "{                                          \n"
                    "   gl_Position = u_mvpMatrix * a_position; \n"
                    "   v_texCoord = a_texCoord;                \n"
                    "}                                          \n";

    char fShaderStr[] =
            "#version 300 es                                     \n"
                    "precision mediump float;                            \n"
                    "in vec2 v_texCoord;                                 \n"
                    "layout(location = 0) out vec4 outColor;             \n"
                    "uniform float s_light;                              \n"
                    "uniform sampler2D s_baseMap;                        \n"
                    "uniform sampler2D s_lightMap;                       \n"
                    "void main()                                         \n"
                    "{                                                   \n"
                    "  vec4 baseColor;                                   \n"
                    "  vec4 lightColor;                                  \n"
                    "                                                    \n"
                    "  baseColor = texture( s_baseMap, v_texCoord );     \n"
                    "  lightColor = texture( s_lightMap, v_texCoord );   \n"
                    "  outColor = baseColor * (lightColor + s_light);    \n"
                    "}                                                   \n";

    // Load the shaders and get a linked program object
    userData->programObject = loadProgram(vShaderStr, fShaderStr);

    // Get the sampler location
    userData->baseMapLoc = glGetUniformLocation(userData->programObject, "s_baseMap");
    userData->lightMapLoc = glGetUniformLocation(userData->programObject, "s_lightMap");

    // Load the textures
    userData->baseMapTexId = LoadTexture(esContext->app->activity->assetManager, "basemap.tga");
    userData->lightMapTexId = LoadTexture(esContext->app->activity->assetManager, "lightmap.tga");

    if (userData->baseMapTexId == 0 || userData->lightMapTexId == 0) {
        return GL_FALSE;
    }

    userData->light = glGetUniformLocation(userData->programObject, "s_light");

    // Get the uniform locations
    userData->mvpLoc = glGetUniformLocation(userData->programObject, "u_mvpMatrix");

    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    return GL_TRUE;
}

void updatePosition(Engine *engine, int quadrant) {
    UserData *userData = (UserData *) engine->userData;
    ESMatrix perspective;
    ESMatrix modelview;

    // Generate a perspective matrix with a 60 degree FOV
    esMatrixLoadIdentity(&perspective);

    // Generate a model view matrix to rotate/translate the cube
    esMatrixLoadIdentity(&modelview);

    float x = (0x1 & quadrant) != 0 ? 1.0 : 0.0;
    float y = (0x2 & quadrant) != 0 ? -1.0 : 0.0;

    // Translate away from the viewer
    esTranslate(&modelview, x, y, 0.0);

    // Compute the final MVP by multiplying the
    // modevleiw and perspective matrices together
    esMatrixMultiply(&userData->mvpMatrix, &modelview, &perspective);
}

///
// Draw a triangle using the shader pair created in Init()
//
void Draw(Engine *esContext) {
    UserData *userData = (UserData *) esContext->userData;
    GLfloat vVertices[] = {-1.0f, 1.0f, 0.0f,  // Position 0
                           0.0f, 0.0f,        // TexCoord 0
                           -1.0f, 0.0f, 0.0f,  // Position 1
                           0.0f, 1.0f,        // TexCoord 1
                           0.0f, 0.0f, 0.0f,  // Position 2
                           1.0f, 1.0f,        // TexCoord 2
                           0.0f, 1.0f, 0.0f,  // Position 3
                           1.0f, 0.0f         // TexCoord 3
    };
    GLushort indices[] = {0, 1, 2, 0, 2, 3};

    // Set the viewport
    glViewport(0, 0, esContext->width, esContext->height);

    // Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT);

    // Use the program object
    glUseProgram(userData->programObject);

    // Bind the base map
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, userData->baseMapTexId);

    // Set the base map sampler to texture unit to 0
    glUniform1i(userData->baseMapLoc, 0);

    // Bind the light map
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, userData->lightMapTexId);

    // Set the light map sampler to texture unit 1
    glUniform1i(userData->lightMapLoc, 1);

    // Load the vertex position
    glVertexAttribPointer(0, 3, GL_FLOAT,
                          GL_FALSE, 5 * sizeof(GLfloat), vVertices);
    // Load the texture coordinate
    glVertexAttribPointer(1, 2, GL_FLOAT,
                          GL_FALSE, 5 * sizeof(GLfloat), &vVertices[3]);

    glEnableVertexAttribArray(0);
    glEnableVertexAttribArray(1);

    for (int i = 0; i < 4; i++) {
        updatePosition(esContext, i);
        // Load the MVP matrix
        glUniform1f(userData->light, 0.1 * (i + 1));
        glUniformMatrix4fv(userData->mvpLoc, 1, GL_FALSE, (GLfloat *) &userData->mvpMatrix.m[0][0]);
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
    }

}

///
// Cleanup
//
void ShutDown(Engine *esContext) {
    UserData *userData = (UserData *) esContext->userData;

    // Delete texture object
    glDeleteTextures(1, &userData->baseMapTexId);
    glDeleteTextures(1, &userData->lightMapTexId);

    // Delete program object
    glDeleteProgram(userData->programObject);
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
