#undef  LOG_TAG
#define LOG_TAG "VboAndNonVbo"

#include <androidnative/NativeMain.h>
#include <androidnative/ShaderLoader.h>

typedef struct {
    // Handle to a program object
    GLuint programObject;

    // VertexBufferObject Ids
    GLuint vboIds[3];

} UserData;


#define VERTEX_POS_SIZE       3 // x, y and z
#define VERTEX_COLOR_SIZE     4 // r, g, b, and a

#define VERTEX_POS_INDX       0
#define VERTEX_COLOR_INDX     1


int Init(Engine *engine) {
    UserData *userData = (UserData *) engine->userData;
    const char vShaderStr[] =
            "#version 300 es                            \n"
                    "layout(location = 0) in vec4 a_position;   \n"
                    "layout(location = 1) in vec4 a_color;      \n"
                    "out vec4 v_color;                          \n"
                    "void main()                                \n"
                    "{                                          \n"
                    "    v_color = a_color;                     \n"
                    "    gl_Position = a_position;              \n"
                    "}";


    const char fShaderStr[] =
            "#version 300 es            \n"
                    "precision mediump float;   \n"
                    "in vec4 v_color;           \n"
                    "out vec4 o_fragColor;      \n"
                    "void main()                \n"
                    "{                          \n"
                    "    o_fragColor = v_color; \n"
                    "}";

    GLuint programObject;

    // Create the program object
    programObject = loadProgram(vShaderStr, fShaderStr);

    if (programObject == 0) {
        return GL_FALSE;
    }

    // Store the program object
    userData->programObject = programObject;
    userData->vboIds[0] = 0;
    userData->vboIds[1] = 0;
    userData->vboIds[2] = 0;


    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    return GL_TRUE;
}

void DrawPrimitiveWithVBOs(Engine *engine,
                           GLint numVertices, GLfloat **vtxBuf,
                           GLint *vtxStrides, GLint numIndices,
                           GLushort *indices) {
    UserData *userData = (UserData *) engine->userData;

    // vboIds[0] - used to store vertex position
    // vboIds[1] - used to store vertex color
    // vboIds[2] - used to store element indices
    if (userData->vboIds[0] == 0 && userData->vboIds[1] == 0 &&
        userData->vboIds[2] == 0) {
        // Only allocate on the first draw
        glGenBuffers(3, userData->vboIds);

        glBindBuffer(GL_ARRAY_BUFFER, userData->vboIds[0]);
        glBufferData(GL_ARRAY_BUFFER, vtxStrides[0] * numVertices,
                     vtxBuf[0], GL_STATIC_DRAW);
        glBindBuffer(GL_ARRAY_BUFFER, userData->vboIds[1]);
        glBufferData(GL_ARRAY_BUFFER, vtxStrides[1] * numVertices,
                     vtxBuf[1], GL_STATIC_DRAW);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, userData->vboIds[2]);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER,
                     sizeof(GLushort) * numIndices,
                     indices, GL_STATIC_DRAW);
    }


    glBindBuffer(GL_ARRAY_BUFFER, userData->vboIds[0]);
    glEnableVertexAttribArray(VERTEX_POS_INDX);
    glVertexAttribPointer(VERTEX_POS_INDX, VERTEX_POS_SIZE,
                          GL_FLOAT, GL_FALSE, vtxStrides[0], 0);

    glBindBuffer(GL_ARRAY_BUFFER, userData->vboIds[1]);
    glEnableVertexAttribArray(VERTEX_COLOR_INDX);
    glVertexAttribPointer(VERTEX_COLOR_INDX,
                          VERTEX_COLOR_SIZE,
                          GL_FLOAT, GL_FALSE, vtxStrides[1], 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, userData->vboIds[2]);

    glDrawElements(GL_TRIANGLES, numIndices,
                   GL_UNSIGNED_SHORT, 0);

    glDisableVertexAttribArray(VERTEX_POS_INDX);
    glDisableVertexAttribArray(VERTEX_COLOR_INDX);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

void Draw(Engine *engine) {
    UserData *userData = (UserData *) engine->userData;

    // 3 vertices, with (x,y,z) ,(r, g, b, a) per-vertex
    GLfloat vertexPos[3 * VERTEX_POS_SIZE] =
            {
                    0.0f, 0.5f, 0.0f,        // v0
                    -0.5f, -0.5f, 0.0f,        // v1
                    0.5f, -0.5f, 0.0f         // v2
            };
    GLfloat color[4 * VERTEX_COLOR_SIZE] =
            {
                    1.0f, 0.0f, 0.0f, 1.0f,   // c0
                    0.0f, 1.0f, 0.0f, 1.0f,   // c1
                    0.0f, 0.0f, 1.0f, 1.0f    // c2
            };
    GLint vtxStrides[2] =
            {
                    VERTEX_POS_SIZE * sizeof(GLfloat),
                    VERTEX_COLOR_SIZE * sizeof(GLfloat)
            };

    // Index buffer data
    GLushort indices[3] = {0, 1, 2};
    GLfloat *vtxBuf[2] = {vertexPos, color};

    glViewport(0, 0, engine->width, engine->height);
    glClear(GL_COLOR_BUFFER_BIT);
    glUseProgram(userData->programObject);

    DrawPrimitiveWithVBOs(engine, 3, vtxBuf,
                          vtxStrides, 3, indices);
}

void Shutdown(Engine *engine) {
    UserData *userData = (UserData *) engine->userData;

    glDeleteProgram(userData->programObject);
    glDeleteBuffers(3, userData->vboIds);
}

int appMain(Engine *engine) {
    engine->userData = (UserData *) malloc(sizeof(UserData));

    if (!Init(engine)) {
        return GL_FALSE;
    }

    registerShutdownFunc(engine, Shutdown);
    registerDrawFunc(engine, Draw);

    return GL_TRUE;
}
