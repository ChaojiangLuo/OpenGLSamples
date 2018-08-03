#undef  LOG_TAG
#define LOG_TAG "MapBuffers"

#include <androidnative/NativeMain.h>
#include <androidnative/ShaderLoader.h>

#include <utils/GLUtils.h>

typedef struct {
    // Handle to a program object
    GLuint programObject;

    // Uniform locations
    GLint mvpLoc;

    // Vertex daata
    GLfloat *vertices;
    GLuint *indices;
    int numIndices;

    // Rotation angle
    GLfloat angle;

    // MVP matrix
    ESMatrix mvpMatrix;
} UserData;

///
// Initialize the shader and program object
//
int Init(Engine *engine) {
    UserData *userData = (UserData *) engine->userData;
    const char vShaderStr[] =
            "#version 300 es                             \n"
                    "uniform mat4 u_mvpMatrix;                   \n"
                    "layout(location = 0) in vec4 a_position;    \n"
                    "layout(location = 1) in vec4 a_color;       \n"
                    "out vec4 v_color;                           \n"
                    "void main()                                 \n"
                    "{                                           \n"
                    "   v_color = a_color;                       \n"
                    "   gl_Position = u_mvpMatrix * a_position;  \n"
                    "}                                           \n";

    const char fShaderStr[] =
            "#version 300 es                                \n"
                    "precision mediump float;                       \n"
                    "in vec4 v_color;                               \n"
                    "layout(location = 0) out vec4 outColor;        \n"
                    "void main()                                    \n"
                    "{                                              \n"
                    "  outColor = v_color;                          \n"
                    "}                                              \n";

    // Load the shaders and get a linked program object
    userData->programObject = loadProgram(vShaderStr, fShaderStr);

    // Get the uniform locations
    userData->mvpLoc = glGetUniformLocation(userData->programObject, "u_mvpMatrix");

    // Generate the vertex data
    userData->numIndices = esGenCube(1.0, &userData->vertices,
                                     NULL, NULL, &userData->indices);

    // Starting rotation angle for the cube
    userData->angle = 45.0f;

    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    return GL_TRUE;
}


///
// Update MVP matrix based on time
//
void Update(Engine *engine, float deltaTime) {
    UserData *userData = (UserData *) engine->userData;
    ESMatrix perspective;
    ESMatrix modelview;
    float aspect;

    // Compute a rotation angle based on time to rotate the cube
    userData->angle += (deltaTime * 40.0f);

    if (userData->angle >= 360.0f) {
        userData->angle -= 360.0f;
    }

    // Compute the window aspect ratio
    aspect = (GLfloat) engine->width / (GLfloat) engine->height;

    // Generate a perspective matrix with a 60 degree FOV
    esMatrixLoadIdentity(&perspective);
    esPerspective(&perspective, 60.0f, aspect, 1.0f, 20.0f);

    // Generate a model view matrix to rotate/translate the cube
    esMatrixLoadIdentity(&modelview);

    // Translate away from the viewer
    esTranslate(&modelview, 0.0, 0.0, -6.0);

    // Rotate the cube
    esRotate(&modelview, userData->angle, 1.0, 0.0, 1.0);

    // Compute the final MVP by multiplying the
    // modevleiw and perspective matrices together
    esMatrixMultiply(&userData->mvpMatrix, &modelview, &perspective);
}

///
// Draw a triangle using the shader pair created in Init()
//
void Draw(Engine *engine) {
    UserData *userData = (UserData *) engine->userData;

    // Set the viewport
    glViewport(0, 0, engine->width, engine->height);

    // Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    // Use the program object
    glUseProgram(userData->programObject);

    // Load the vertex position
    glVertexAttribPointer(0, 3, GL_FLOAT,
                          GL_FALSE, 3 * sizeof(GLfloat), userData->vertices);

    glEnableVertexAttribArray(0);

    // Set the vertex color to red
    glVertexAttrib4f(1, 1.0f, 0.0f, 0.0f, 1.0f);

    // Load the MVP matrix
    glUniformMatrix4fv(userData->mvpLoc, 1, GL_FALSE, (GLfloat *) &userData->mvpMatrix.m[0][0]);

    // Draw the cube
    glDrawElements(GL_TRIANGLES, userData->numIndices, GL_UNSIGNED_INT, userData->indices);
}

///
// Cleanup
//
void Shutdown(Engine *engine) {
    UserData *userData = (UserData *) engine->userData;

    if (userData->vertices != NULL) {
        free(userData->vertices);
    }

    if (userData->indices != NULL) {
        free(userData->indices);
    }

    // Delete program object
    glDeleteProgram(userData->programObject);
}

int appMain(Engine *engine) {
    engine->userData = (UserData *) malloc(sizeof(UserData));

    if (!Init(engine)) {
        return GL_FALSE;
    }

    registerShutdownFunc(engine, Shutdown);
    registerUpdateFunc(engine, Update);
    registerDrawFunc(engine, Draw);

    return GL_TRUE;
}

