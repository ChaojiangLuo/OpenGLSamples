//
// Created by luocj on 18-7-13.
//

#ifndef OPENGLSAMPLES_EGLUTILS_H
#define OPENGLSAMPLES_EGLUTILS_H

#ifndef LOG_TAG
#define LOG_TAG "EGL_Error"
#endif

#include <EGL/egl.h>

#include <androidnative/MyLog.h>

static void checkEGLErrors() {
    EGLint error = eglGetError();
    if (error && error != EGL_SUCCESS)
        MyLOGE("EGL Error: 0x%04x\n", (int) error);
}

const char *egl_strerror(unsigned err) {
    switch (err) {
        case EGL_SUCCESS:
            return "SUCCESS";
        case EGL_NOT_INITIALIZED:
            return "NOT INITIALIZED";
        case EGL_BAD_ACCESS:
            return "BAD ACCESS";
        case EGL_BAD_ALLOC:
            return "BAD ALLOC";
        case EGL_BAD_ATTRIBUTE:
            return "BAD_ATTRIBUTE";
        case EGL_BAD_CONFIG:
            return "BAD CONFIG";
        case EGL_BAD_CONTEXT:
            return "BAD CONTEXT";
        case EGL_BAD_CURRENT_SURFACE:
            return "BAD CURRENT SURFACE";
        case EGL_BAD_DISPLAY:
            return "BAD DISPLAY";
        case EGL_BAD_MATCH:
            return "BAD MATCH";
        case EGL_BAD_NATIVE_PIXMAP:
            return "BAD NATIVE PIXMAP";
        case EGL_BAD_NATIVE_WINDOW:
            return "BAD NATIVE WINDOW";
        case EGL_BAD_PARAMETER:
            return "BAD PARAMETER";
        case EGL_BAD_SURFACE:
            return "BAD_SURFACE";
            //    case EGL_CONTEXT_LOST: return "CONTEXT LOST";
        default:
            return "UNKNOWN";
    }
}

void checkEGLApiErrors(const char *name) {
    unsigned err = eglGetError();
    if (err != EGL_SUCCESS) {
        MyLOGE("%s(): egl error 0x%x (%s)\n", name, err, egl_strerror(err));
    }
}

#endif //OPENGLSAMPLES_EGLUTILS_H
