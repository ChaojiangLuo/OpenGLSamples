#include <stdlib.h>

#include <androidnative/NativeMain.h>
#include <androidnative/ShaderLoader.h>

#include <utils/GLUtils.h>

typedef struct {
    // Handle to a program object
    GLuint programObject;

    // Handle to a framebuffer object
    GLuint fbo;

    // Texture handle
    GLuint colorTexId[4];

    // Texture size
    GLsizei textureWidth;
    GLsizei textureHeight;

} UserData;

///
// Initialize the framebuffer object and MRTs
//
int InitFBO(Engine *esContext) {
    UserData *userData = static_cast<UserData *> (esContext->userData);
    int i;
    GLint defaultFramebuffer = 0;
    const GLenum attachments[4] =
            {
                    GL_COLOR_ATTACHMENT0,
                    GL_COLOR_ATTACHMENT1,
                    GL_COLOR_ATTACHMENT2,
                    GL_COLOR_ATTACHMENT3
            };

    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFramebuffer);

    // Setup fbo
    glGenFramebuffers(1, &userData->fbo);
    glBindFramebuffer(GL_FRAMEBUFFER, userData->fbo);

    // Setup four output buffers and attach to fbo
    userData->textureHeight = userData->textureWidth = 400;
    glGenTextures(4, &userData->colorTexId[0]);
    for (i = 0; i < 4; ++i) {
        glBindTexture(GL_TEXTURE_2D, userData->colorTexId[i]);

        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA,
                     userData->textureWidth, userData->textureHeight,
                     0, GL_RGBA, GL_UNSIGNED_BYTE, NULL);

        // Set the filtering mode
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glFramebufferTexture2D(GL_DRAW_FRAMEBUFFER, attachments[i],
                               GL_TEXTURE_2D, userData->colorTexId[i], 0);
    }

    glDrawBuffers(4, attachments);

    if (GL_FRAMEBUFFER_COMPLETE != glCheckFramebufferStatus(GL_FRAMEBUFFER)) {
        return GL_FALSE;
    }

    // Restore the original framebuffer
    glBindFramebuffer(GL_FRAMEBUFFER, defaultFramebuffer);

    return GL_TRUE;
}

///
// Initialize the shader and program object
//
int Init(Engine *esContext) {
    UserData *userData = static_cast<UserData *> (esContext->userData);
    char vShaderStr[] =
            "#version 300 es                            \n"
            "layout(location = 0) in vec4 a_position;   \n"
            "void main()                                \n"
            "{                                          \n"
            "   gl_Position = a_position;               \n"
            "}                                          \n";

    char fShaderStr[] =
            "#version 300 es                                     \n"
            "precision mediump float;                            \n"
            "layout(location = 0) out vec4 fragData0;            \n"
            "layout(location = 1) out vec4 fragData1;            \n"
            "layout(location = 2) out vec4 fragData2;            \n"
            "layout(location = 3) out vec4 fragData3;            \n"
            "void main()                                         \n"
            "{                                                   \n"
            "  // first buffer will contain red color            \n"
            "  fragData0 = vec4 ( 1, 0, 0, 1 );                  \n"
            "                                                    \n"
            "  // second buffer will contain green color         \n"
            "  fragData1 = vec4 ( 0, 1, 0, 1 );                  \n"
            "                                                    \n"
            "  // third buffer will contain blue color           \n"
            "  fragData2 = vec4 ( 0, 0, 1, 1 );                  \n"
            "                                                    \n"
            "  // fourth buffer will contain gray color          \n"
            "  fragData3 = vec4 ( 0.5, 0.5, 0.5, 1 );            \n"
            "}                                                   \n";

    // Load the shaders and get a linked program object
    userData->programObject = loadProgram(vShaderStr, fShaderStr);

    InitFBO(esContext);

    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    return GL_TRUE;
}

///
// Draw a quad and output four colors per pixel
//
void DrawGeometry(Engine *esContext) {
    UserData *userData = static_cast<UserData *> (esContext->userData);
    GLfloat vVertices[] = {-1.0f, 1.0f, 0.0f,
                           -1.0f, -1.0f, 0.0f,
                           1.0f, -1.0f, 0.0f,
                           1.0f, 1.0f, 0.0f,
    };
    GLushort indices[] = {0, 1, 2, 0, 2, 3};

    // Set the viewport
    glViewport(0, 0, esContext->width, esContext->height);

    // Clear the color buffer
    glClear(GL_COLOR_BUFFER_BIT);

    // Use the program object
    glUseProgram(userData->programObject);

    // Load the vertex position
    glVertexAttribPointer(0, 3, GL_FLOAT,
                          GL_FALSE, 3 * sizeof(GLfloat), vVertices);
    glEnableVertexAttribArray(0);

    // Draw a quad
    glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_SHORT, indices);
}

///
// Copy MRT output buffers to screen
//
void BlitTextures(Engine *esContext) {
    UserData *userData = static_cast<UserData *> (esContext->userData);

    // set the fbo for reading
    glBindFramebuffer(GL_READ_FRAMEBUFFER, userData->fbo);

    // Copy the output red buffer to lower left quadrant
    glReadBuffer(GL_COLOR_ATTACHMENT0);
    glBlitFramebuffer(0, 0, userData->textureWidth, userData->textureHeight,
                      0, 0, esContext->width / 2, esContext->height / 2,
                      GL_COLOR_BUFFER_BIT, GL_LINEAR);

    // Copy the output green buffer to lower right quadrant
    glReadBuffer(GL_COLOR_ATTACHMENT1);
    glBlitFramebuffer(0, 0, userData->textureWidth, userData->textureHeight,
                      esContext->width / 2, 0, esContext->width, esContext->height / 2,
                      GL_COLOR_BUFFER_BIT, GL_LINEAR);

    // Copy the output blue buffer to upper left quadrant
    glReadBuffer(GL_COLOR_ATTACHMENT2);
    glBlitFramebuffer(0, 0, userData->textureWidth, userData->textureHeight,
                      0, esContext->height / 2, esContext->width / 2, esContext->height,
                      GL_COLOR_BUFFER_BIT, GL_LINEAR);

    // Copy the output gray buffer to upper right quadrant
    glReadBuffer(GL_COLOR_ATTACHMENT3);
    glBlitFramebuffer(0, 0, userData->textureWidth, userData->textureHeight,
                      esContext->width / 2, esContext->height / 2, esContext->width,
                      esContext->height,
                      GL_COLOR_BUFFER_BIT, GL_LINEAR);
}

///
// Render to MRTs and screen
//
void Draw(Engine *esContext) {
    UserData *userData = static_cast<UserData *> (esContext->userData);
    GLint defaultFramebuffer = 0;
    const GLenum attachments[4] =
            {
                    GL_COLOR_ATTACHMENT0,
                    GL_COLOR_ATTACHMENT1,
                    GL_COLOR_ATTACHMENT2,
                    GL_COLOR_ATTACHMENT3
            };

    glGetIntegerv(GL_FRAMEBUFFER_BINDING, &defaultFramebuffer);

    // FIRST: use MRTs to output four colors to four buffers
    glBindFramebuffer(GL_FRAMEBUFFER, userData->fbo);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    glDrawBuffers(4, attachments);
    DrawGeometry(esContext);

    // SECOND: copy the four output buffers into four window quadrants
    // with framebuffer blits

    // Restore the default framebuffer
    glBindFramebuffer(GL_DRAW_FRAMEBUFFER, defaultFramebuffer);
    BlitTextures(esContext);
}

///
// Cleanup
//
void ShutDown(Engine *esContext) {
    UserData *userData = static_cast<UserData *> (esContext->userData);

    // Delete texture objects
    glDeleteTextures(4, userData->colorTexId);

    // Delete fbo
    glDeleteFramebuffers(1, &userData->fbo);

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
