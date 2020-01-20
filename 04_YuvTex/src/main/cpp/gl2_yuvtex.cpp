#undef  LOG_TAG
#define LOG_TAG "MultiTexture"

#include <androidnative/NativeMain.h>
#include <androidnative/ShaderLoader.h>

#include <utils/GLUtils.h>

typedef struct {
    // Handle to a program object
    GLuint programObject;

    GLint gvPositionHandle;
    GLint gYuvTexSamplerHandle;

    int yuvTexWidth = 608;
    int yuvTexHeight = 480;
    int yuvTexUsage = AHARDWAREBUFFER_USAGE_GPU_SAMPLED_IMAGE | AHARDWAREBUFFER_USAGE_CPU_WRITE_RARELY;
    int yuvTexFormat = 842094169;

    ANativeWindow* yuvTexBuffer;

} UserData;

bool setupYuvTexSurface(EGLDisplay dpy, Engine *esContext) {
    UserData *userData = (UserData *) esContext->userData;
    int blockWidth = userData->yuvTexWidth > 16 ? userData->yuvTexWidth / 16 : 1;
    int blockHeight = userData->yuvTexHeight > 16 ? userData->yuvTexHeight / 16 : 1;
    userData->yuvTexBuffer = new ANativeWindow();
    ANativeWindow_setBuffersGeometry(userData->yuvTexBuffer, userData->yuvTexWidth, userData->yuvTexHeight, userData->yuvTexFormat);

    int yuvTexStrideY = yuvTexBuffer->getStride();
    int yuvTexOffsetV = yuvTexStrideY * yuvTexHeight;
    int yuvTexStrideV = (yuvTexStrideY/2 + 0xf) & ~0xf;
    int yuvTexOffsetU = yuvTexOffsetV + yuvTexStrideV * yuvTexHeight/2;
    int yuvTexStrideU = yuvTexStrideV;
    char* buf = NULL;
    status_t err = yuvTexBuffer->lock(GRALLOC_USAGE_SW_WRITE_OFTEN, (void**)(&buf));
    if (err != 0) {
        fprintf(stderr, "yuvTexBuffer->lock(...) failed: %d\n", err);
        return false;
    }
    for (int x = 0; x < yuvTexWidth; x++) {
        for (int y = 0; y < yuvTexHeight; y++) {
            int parityX = (x / blockWidth) & 1;
            int parityY = (y / blockHeight) & 1;
            unsigned char intensity = (parityX ^ parityY) ? 63 : 191;
            buf[yuvTexOffsetY + (y * yuvTexStrideY) + x] = intensity;
            if (x < yuvTexWidth / 2 && y < yuvTexHeight / 2) {
                buf[yuvTexOffsetU + (y * yuvTexStrideU) + x] = intensity;
                if (yuvTexSameUV) {
                    buf[yuvTexOffsetV + (y * yuvTexStrideV) + x] = intensity;
                } else if (x < yuvTexWidth / 4 && y < yuvTexHeight / 4) {
                    buf[yuvTexOffsetV + (y*2 * yuvTexStrideV) + x*2 + 0] =
                    buf[yuvTexOffsetV + (y*2 * yuvTexStrideV) + x*2 + 1] =
                    buf[yuvTexOffsetV + ((y*2+1) * yuvTexStrideV) + x*2 + 0] =
                    buf[yuvTexOffsetV + ((y*2+1) * yuvTexStrideV) + x*2 + 1] = intensity;
                }
            }
        }
    }

    err = yuvTexBuffer->unlock();
    if (err != 0) {
        fprintf(stderr, "yuvTexBuffer->unlock() failed: %d\n", err);
        return false;
    }

    EGLClientBuffer clientBuffer = (EGLClientBuffer)yuvTexBuffer->getNativeBuffer();
    EGLImageKHR img = eglCreateImageKHR(dpy, EGL_NO_CONTEXT, EGL_NATIVE_BUFFER_ANDROID,
                                        clientBuffer, 0);
    checkEglError("eglCreateImageKHR");
    if (img == EGL_NO_IMAGE_KHR) {
        return false;
    }

    glGenTextures(1, &yuvTex);
    checkGlError("glGenTextures");
    glBindTexture(GL_TEXTURE_EXTERNAL_OES, yuvTex);
    checkGlError("glBindTexture");
    glEGLImageTargetTexture2DOES(GL_TEXTURE_EXTERNAL_OES, (GLeglImageOES)img);
    checkGlError("glEGLImageTargetTexture2DOES");

    return true;
}

///
// Initialize the shader and program object
//
int Init(Engine *esContext) {
    UserData *userData = (UserData *) esContext->userData;
    char gVertexShader[] = "attribute vec4 vPosition;\n"
                           "varying vec2 yuvTexCoords;\n"
                           "void main() {\n"
                           "  yuvTexCoords = vPosition.xy + vec2(0.5, 0.5);\n"
                           "  gl_Position = vPosition;\n"
                           "}\n";

    char gFragmentShader[] = "#extension GL_OES_EGL_image_external : require\n"
                        "precision mediump float;\n"
                        "uniform samplerExternalOES yuvTexSampler;\n"
                        "varying vec2 yuvTexCoords;\n"
                        "void main() {\n"
                        "  gl_FragColor = texture2D(yuvTexSampler, yuvTexCoords);\n"
                        "}\n";

    // Load the shaders and get a linked program object
    userData->programObject = loadProgram(gVertexShader, gFragmentShader);


    glClearColor(1.0f, 1.0f, 1.0f, 0.0f);
    return GL_TRUE;
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