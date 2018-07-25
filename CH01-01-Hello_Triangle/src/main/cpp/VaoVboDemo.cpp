//
// Created by luocj on 18-7-16.
// https://blog.csdn.net/xiajun07061225/article/details/7628146
//
#define LOG_TAG "VaoVboDemo"

#include <GLES3/gl3.h>
#include <android_native_app_glue.h>

#include <androidnative/NativeMain.h>

#include <androidnative/ShaderLoader.h>
#include <androidnative/MyLog.h>
#include "VaoVboDemo.h"

GLuint vShader, fShader;//顶点着色器对象
GLint programHandle;
GLuint vaoHandle;//vertex array object

//顶点位置数组
float positionData[] = {
        -0.8f, -0.8f, 0.0f,
        0.8f, -0.8f, 0.0f,
        0.0f, 0.8f, 0.0f};
//颜色数组
float colorData[] = {
        1.0f, 0.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 0.0f, 1.0f};

void initShader(struct Engine *engine) {
    const GLubyte *renderer = glGetString(GL_RENDERER);
    const GLubyte *vendor = glGetString(GL_VENDOR);
    const GLubyte *version = glGetString(GL_VERSION);
    const GLubyte *glslVersion = glGetString(GL_SHADING_LANGUAGE_VERSION);
    GLint major, minor;
    glGetIntegerv(GL_MAJOR_VERSION, &major);
    glGetIntegerv(GL_MINOR_VERSION, &minor);
    MyLOGI("GL Vendor    : %s", vendor);
    MyLOGI("GL Renderer  : %s", renderer);
    MyLOGI("GL Version (string)  : %s", version);
    MyLOGI("GL Version (integer) : %d.%d", major, minor);
    MyLOGI("GLSL Version : %s", glslVersion);

    char *vShaderSource = readShaderSrcFile("shaders/basic.vert",
                                            engine->app->activity->assetManager);
    char *fShaderSource = readShaderSrcFile("shaders/basic.frag",
                                            engine->app->activity->assetManager);

    fShader = LoadShader(GL_FRAGMENT_SHADER, fShaderSource);
    if (!fShader) {
        MyLOGE("ERROR : LoadShader fragment shader failed");
        return;
    }

    vShader = LoadShader(GL_VERTEX_SHADER, vShaderSource);
    if (!vShader) {
        MyLOGE("ERROR : LoadShader vertex shader failed");
        return;
    }

    //3、链接着色器对象
    //创建着色器程序
    programHandle = glCreateProgram();
    if (!programHandle) {
        MyLOGE("ERROR : create program failed");
        return;
    }
    //将着色器程序链接到所创建的程序中
    glAttachShader(programHandle, vShader);
    glAttachShader(programHandle, fShader);
    //将这些对象链接成一个可执行程序
    glLinkProgram(programHandle);
    //查询链接的结果
    GLint linkStatus;
    glGetProgramiv(programHandle, GL_LINK_STATUS, &linkStatus);
    if (linkStatus != GL_TRUE) {
        MyLOGE("ERROR : link shader program failed");

        GLint logLen;
        glGetProgramiv(programHandle, GL_INFO_LOG_LENGTH,
                       &logLen);
        if (logLen > 1) {
            char *log = (char *) malloc(logLen);
            GLsizei written;
            glGetProgramInfoLog(programHandle, logLen,
                                &written, log);
            MyLOGE("Program log : %s", log);
            free(log);
        }
        glDetachShader(programHandle, vShader);
        glDetachShader(programHandle, fShader);
        glDeleteProgram(programHandle);
        return;
    }
    //链接成功，在OpenGL管线中使用渲染程序
    glUseProgram(programHandle);
}

void initVBO() {
    // Create and populate the buffer objects
    GLuint vboHandles[2];
    glGenBuffers(2, vboHandles);
    GLuint positionBufferHandle = vboHandles[0];
    GLuint colorBufferHandle = vboHandles[1];

    //绑定VBO以供使用
    glBindBuffer(GL_ARRAY_BUFFER, positionBufferHandle);
    //加载数据到VBO
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float),
                 positionData, GL_STATIC_DRAW);

    //绑定VBO以供使用
    glBindBuffer(GL_ARRAY_BUFFER, colorBufferHandle);
    //加载数据到VBO
    glBufferData(GL_ARRAY_BUFFER, 9 * sizeof(float),
                 colorData, GL_STATIC_DRAW);

    glGenVertexArrays(1, &vaoHandle);
    glBindVertexArray(vaoHandle);

    glEnableVertexAttribArray(0);//顶点坐标
    glEnableVertexAttribArray(1);//顶点颜色

    //调用glVertexAttribPointer之前需要进行绑定操作
    glBindBuffer(GL_ARRAY_BUFFER, positionBufferHandle);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte *) NULL);

    glBindBuffer(GL_ARRAY_BUFFER, colorBufferHandle);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, (GLubyte *) NULL);
}

int appMain(struct Engine *engine) {
    initShader(engine);

    initVBO();

    glClearColor(0.0, 0.0, 0.0, 0.0);

    registerDrawFunc(engine, appDisplay);
    registerShutdownFunc(engine, appDeinit);

    return 0;
}

void appDisplay(struct Engine *engine) {
    glClear(GL_COLOR_BUFFER_BIT);

    //使用VAO、VBO绘制
    glBindVertexArray(vaoHandle);
    glDrawArrays(GL_TRIANGLES, 0, 3);
    glBindVertexArray(0);
}

void appDeinit(struct Engine *engine) {
    glDeleteShader(vShader);
    glDeleteShader(fShader);
    glDeleteProgram(programHandle);
    glUseProgram(0);
}