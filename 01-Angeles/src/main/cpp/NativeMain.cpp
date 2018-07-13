//
// Created by luocj on 18-7-13.
//

#undef  LOG_TAG
#define LOG_TAG "OpenGL2Basic"

#include <android/sensor.h>
#include <android_native_app_glue.h>

#include <cassert>
#include <cstdlib>
#include <cstring>
#include <errno.h>

#include <initializer_list>

#include <EGL/egl.h>
#include <GLES/gl.h>

#include "EGLUtils.h"
#include "GLUtils.h"
#include "MyLog.h"
#include "NativeMain.h"

#include "demo.h"

/**
 * Initialize an EGL context for the current display.
 */
static int initDisplay(struct Engine *engine) {
    // initialize OpenGL ES and EGL

    /*
     * Here specify the attributes of the desired configuration.
     * Below, we select an EGLConfig with at least 8 bits per color
     * component compatible with on-screen windows
     */
    const EGLint configAttribs[] = {
            EGL_DEPTH_SIZE, 16,
            EGL_SAMPLE_BUFFERS, engine->samples ? 1 : 0,
            EGL_SAMPLES, engine->samples,
            EGL_NONE
    };

    EGLint majorVersion;
    EGLint minorVersion;
    EGLContext context;
    EGLConfig config;
    EGLSurface surface;
    EGLint w, h, format = WINDOW_FORMAT_RGBA_8888;
    EGLDisplay display;

    display = eglGetDisplay(EGL_DEFAULT_DISPLAY);
    eglInitialize(display, &majorVersion, &minorVersion);

    EGLint numConfigs = -1, n = 0;

    // Get all the "potential match" configs...
    if (eglGetConfigs(display, NULL, 0, &numConfigs) == EGL_FALSE) {
        MyLOGE("%s Unable to eglMakeCurrent", __FUNCTION__);
        return -EINVAL;
    }

    EGLConfig *const configs = new EGLConfig[numConfigs];
    if (eglChooseConfig(display, configAttribs, configs, numConfigs, &n) == EGL_FALSE) {
        return -EINVAL;
    }

    int i;
    for (i = 0; i < n; i++) {
        EGLint nativeVisualId = 0;
        eglGetConfigAttrib(display, configs[i], EGL_NATIVE_VISUAL_ID, &nativeVisualId);
        if (nativeVisualId > 0 && format == nativeVisualId) {
            config = configs[i];
            break;
        }
    }

    if (i == n) {
        config = configs[0];
    }

    surface = eglCreateWindowSurface(display, config, engine->app->window, NULL);
    checkEGLErrors("eglCreateWindowSurface");
    MyLOGI("surface = %p\n", surface);

    context = eglCreateContext(display, config, NULL, NULL);
    checkEGLErrors("eglCreateContext");
    MyLOGI("context = %p\n", context);

    if (eglMakeCurrent(display, surface, surface, context) == EGL_FALSE) {
        MyLOGE("%s Unable to eglMakeCurrent", __FUNCTION__);
        return -EINVAL;
    }

    eglQuerySurface(display, surface, EGL_WIDTH, &w);
    eglQuerySurface(display, surface, EGL_HEIGHT, &h);

    engine->display = display;
    engine->context = context;
    engine->surface = surface;
    engine->width = w;
    engine->height = h;

    // Check openGL on the system
    auto opengl_info = {GL_VENDOR, GL_RENDERER, GL_VERSION, GL_EXTENSIONS};
    for (auto name : opengl_info) {
        auto info = glGetString(name);
        MyLOGI("OpenGL Info: %s", info);
    }

    if (engine->samples == 0) {
        // GL_MULTISAMPLE is enabled by default
        glDisable(GL_MULTISAMPLE);
    }
    return 0;
}

/**
 * Just the current frame in the display.
 */
int drawInitFrame(struct Engine *engine) {
    if (engine->display == NULL) {
        MyLOGW("%s No display", __FUNCTION__);
        return -EINVAL;
    }

    // Just fill the screen with a color.
    glClearColor(0.6f, 0.6f, 0.6f, 0.5f);
    glClear(GL_COLOR_BUFFER_BIT);

    eglSwapBuffers(engine->display, engine->surface);

    return 0;
}

/**
 * Just the current frame in the display.
 */
int drawAppFrame(struct Engine *engine) {
    if (engine->display == NULL) {
        MyLOGW("%s No display", __FUNCTION__);
        return -EINVAL;
    }

    struct timeval timeNow;

    gettimeofday(&timeNow, NULL);
    appRender(engine->render, timeNow.tv_sec * 1000 + timeNow.tv_usec / 1000,
              engine->width, engine->height);
    checkGLErrors();
    eglSwapBuffers(engine->display, engine->surface);
    checkEGLErrors();
    engine->frameInfo.frameCount++;

    return 0;
}

/**
 * Just the current frame in the display.
 */
int drawFrame(struct Engine *engine) {
    if (engine->display == NULL) {
        MyLOGW("%s No display", __FUNCTION__);
        return -EINVAL;
    }

    return drawAppFrame(engine);
}

/**
 * Tear down the EGL context currently associated with the display.
 */
static void termDisplay(struct Engine *engine) {
    if (engine->display != EGL_NO_DISPLAY) {
        eglMakeCurrent(engine->display, EGL_NO_SURFACE, EGL_NO_SURFACE, EGL_NO_CONTEXT);
        if (engine->context != EGL_NO_CONTEXT) {
            eglDestroyContext(engine->display, engine->context);
        }
        if (engine->surface != EGL_NO_SURFACE) {
            eglDestroySurface(engine->display, engine->surface);
        }
        eglTerminate(engine->display);
    }
    engine->display = EGL_NO_DISPLAY;
    engine->context = EGL_NO_CONTEXT;
    engine->surface = EGL_NO_SURFACE;
}

/**
 * Process the next input event.
 */
static int32_t engine_handle_input(struct android_app *app, AInputEvent *event) {
    struct Engine *engine = (struct Engine *) app->userData;
    if (AInputEvent_getType(event) == AINPUT_EVENT_TYPE_MOTION) {
        MyLOGD("engine_handle_input-%d-AINPUT_EVENT_TYPE_MOTION", AINPUT_EVENT_TYPE_MOTION);

        switch (AMotionEvent_getAction(event)) {
            case AMOTION_EVENT_ACTION_DOWN:
                break;

            case AMOTION_EVENT_ACTION_UP:
                engine->render->isAlive = !engine->render->isAlive;
                break;
        }

        return 1;
    }
    MyLOGD("engine_handle_input-%d", AInputEvent_getType(event));
    return 0;
}

/**
 * Process the next main command.
 */
static void engine_handle_cmd(struct android_app *app, int32_t cmd) {
    struct Engine *engine = (struct Engine *) app->userData;
    switch (cmd) {
        case APP_CMD_SAVE_STATE:
            MyLOGD("processCommand-%d-APP_CMD_SAVE_STATE", APP_CMD_SAVE_STATE);
            // The system has asked us to save our current state.  Do so.
            engine->app->savedState = malloc(sizeof(struct Saved_State));
            *((struct Saved_State *) engine->app->savedState) = engine->savedState;
            engine->app->savedStateSize = sizeof(struct Saved_State);
            break;

        case APP_CMD_INIT_WINDOW:
            MyLOGD("processCommand-%d-APP_CMD_INIT_WINDOW", APP_CMD_INIT_WINDOW);
            // The window is being shown, get it ready.
            if (engine->app->window != NULL) {
                initDisplay(engine);
                drawInitFrame(engine);
                appInit(engine->render);
            }
            engine->playState = STATE_PREPARED;
            break;

        case APP_CMD_TERM_WINDOW:
            MyLOGD("processCommand-%d-APP_CMD_TERM_WINDOW", APP_CMD_TERM_WINDOW);
            // The window is being hidden or closed, clean it up.
            termDisplay(engine);
            break;

        case APP_CMD_GAINED_FOCUS:
            MyLOGD("processCommand-%d-APP_CMD_GAINED_FOCUS", APP_CMD_GAINED_FOCUS);
            // When our app gains focus, we start monitoring the accelerometer.
            if (engine->accelerometerSensor != NULL) {
                ASensorEventQueue_enableSensor(engine->sensorEventQueue,
                                               engine->accelerometerSensor);
                // We'd like to get 60 events per second (in us).
                ASensorEventQueue_setEventRate(engine->sensorEventQueue,
                                               engine->accelerometerSensor,
                                               (1000L / 60) * 1000);
            }
            break;

        case APP_CMD_LOST_FOCUS:
            MyLOGD("processCommand-%d-APP_CMD_LOST_FOCUS", APP_CMD_LOST_FOCUS);
            // When our app loses focus, we stop monitoring the accelerometer.
            // This is to avoid consuming battery while not being used.
            if (engine->accelerometerSensor != NULL) {
                ASensorEventQueue_disableSensor(engine->sensorEventQueue,
                                                engine->accelerometerSensor);
            }
            // Also stop animating.
            engine->playState = STATE_PAUSED;
            drawInitFrame(engine);
            break;

        case APP_CMD_INPUT_CHANGED:
            MyLOGD("processCommand-%d-APP_CMD_INPUT_CHANGED", cmd);
            break;

        case APP_CMD_WINDOW_RESIZED:
            MyLOGD("processCommand-%d-APP_CMD_WINDOW_RESIZED", cmd);
            break;

        case APP_CMD_WINDOW_REDRAW_NEEDED:
            MyLOGD("processCommand-%d-APP_CMD_WINDOW_REDRAW_NEEDED", cmd);
            break;

        case APP_CMD_CONTENT_RECT_CHANGED:
            MyLOGD("processCommand-%d-APP_CMD_CONTENT_RECT_CHANGED", cmd);
            break;

        case APP_CMD_CONFIG_CHANGED:
            MyLOGD("processCommand-%d-APP_CMD_CONFIG_CHANGED", cmd);
            break;

        case APP_CMD_LOW_MEMORY:
            MyLOGD("processCommand-%d-APP_CMD_LOW_MEMORY", cmd);
            break;

        case APP_CMD_START:
            MyLOGD("processCommand-%d-APP_CMD_START", cmd);
            break;

        case APP_CMD_RESUME:
            MyLOGD("processCommand-%d-APP_CMD_RESUME", cmd);
            break;

        case APP_CMD_PAUSE:
            MyLOGD("processCommand-%d-APP_CMD_PAUSE", cmd);
            break;

        case APP_CMD_STOP:
            MyLOGD("processCommand-%d-APP_CMD_STOP", cmd);
            break;

        case APP_CMD_DESTROY:
            MyLOGD("processCommand-%d-APP_CMD_DESTROY", cmd);
            break;

        default:
            MyLOGD("processCommand-%d-default", cmd);
    }
}

/*
 * AcquireASensorManagerInstance(void)
 *    Workaround ASensorManager_getInstance() deprecation false alarm
 *    for Android-N and before, when compiling with NDK-r15
 */
#include <dlfcn.h>

ASensorManager *AcquireASensorManagerInstance(android_app *app) {

    if (!app)
        return nullptr;

    typedef ASensorManager *(*PF_GETINSTANCEFORPACKAGE)(const char *name);
    void *androidHandle = dlopen("libandroid.so", RTLD_NOW);
    PF_GETINSTANCEFORPACKAGE getInstanceForPackageFunc = (PF_GETINSTANCEFORPACKAGE)
            dlsym(androidHandle, "ASensorManager_getInstanceForPackage");
    if (getInstanceForPackageFunc) {
        JNIEnv *env = nullptr;
        app->activity->vm->AttachCurrentThread(&env, NULL);

        jclass android_content_Context = env->GetObjectClass(app->activity->clazz);
        jmethodID midGetPackageName = env->GetMethodID(android_content_Context,
                                                       "getPackageName",
                                                       "()Ljava/lang/String;");
        jstring packageName = (jstring) env->CallObjectMethod(app->activity->clazz,
                                                              midGetPackageName);

        const char *nativePackageName = env->GetStringUTFChars(packageName, 0);
        ASensorManager *mgr = getInstanceForPackageFunc(nativePackageName);
        env->ReleaseStringUTFChars(packageName, nativePackageName);
        app->activity->vm->DetachCurrentThread();
        if (mgr) {
            dlclose(androidHandle);
            return mgr;
        }
    }

    typedef ASensorManager *(*PF_GETINSTANCE)();
    PF_GETINSTANCE getInstanceFunc = (PF_GETINSTANCE)
            dlsym(androidHandle, "ASensorManager_getInstance");
    // by all means at this point, ASensorManager_getInstance should be available
    assert(getInstanceFunc);
    dlclose(androidHandle);

    return getInstanceFunc();
}


/**
 * This is the main entry point of a native application that is using
 * android_native_app_glue.  It runs in its own thread, with its own
 * event loop for receiving input events and doing other things.
 */
void android_main(struct android_app *app) {
    MyLOGD("android_main start");
    struct Engine engine;

    memset(&engine, 0, sizeof(Engine));
    app->userData = &engine;
    app->onAppCmd = engine_handle_cmd;
    app->onInputEvent = engine_handle_input;

    struct OpenGLRender render;
    memset(&render, 0, sizeof(OpenGLRender));
    engine.render = &render;

    struct FrameInfo frameInfo;
    memset(&frameInfo, 0, sizeof(FrameInfo));
    engine.frameInfo = frameInfo;

    engine.isAlive = 1;
    engine.playState = STATE_IDLE;

    engine.app = app;

    // Prepare to monitor accelerometer
    engine.sensorManager = AcquireASensorManagerInstance(app);
    engine.accelerometerSensor = ASensorManager_getDefaultSensor(
            engine.sensorManager,
            ASENSOR_TYPE_ACCELEROMETER);
    engine.sensorEventQueue = ASensorManager_createEventQueue(
            engine.sensorManager,
            app->looper, LOOPER_ID_USER,
            NULL, NULL);

    if (app->savedState != NULL) {
        // We are starting with a previous saved state; restore from it.
        engine.savedState = *(struct Saved_State *) app->savedState;
    }

    struct timeval timeTemp;
    frameInfo.frameCount = 0;
    gettimeofday(&timeTemp, NULL);
    frameInfo.totalTime = timeTemp.tv_usec / 1000000.0 + timeTemp.tv_sec;

    // loop waiting for stuff to do.
    while (engine.isAlive) {
        // Read all pending events.
        int ident;
        int events;
        struct android_poll_source *source;
        MyLOGI("android_main while 1");

        // If not animating, we will block forever waiting for events.
        // If animating, we loop until all events are read, then continue
        // to draw the next frame of animation.
        while ((ident = ALooper_pollAll(engine.playState ? 0 : -1, NULL, &events,
                                        (void **) &source)) >= 0) {
            MyLOGI("android_main while ALooper_pollAll");
            // Process this event.
            if (source != NULL) {
                MyLOGI("android_main while source->process");
                source->process(app, source);
            }

            // If a sensor has data, process it now.
            if (ident == LOOPER_ID_USER) {
                MyLOGI("android_main while LOOPER_ID_USER");
                if (engine.accelerometerSensor != NULL) {
                    ASensorEvent event;
                    while (ASensorEventQueue_getEvents(engine.sensorEventQueue,
                                                       &event, 1) > 0) {
                        MyLOGI("android_main accelerometer: x=%f y=%f z=%f",
                               event.acceleration.x, event.acceleration.y,
                               event.acceleration.z);
                    }
                }
            }

            // Check if we are exiting.
            if (app->destroyRequested != 0) {
                MyLOGD("android_main while engine_term_display");
                engine.isAlive = 0;
                break;
            }
        }

        if (render.isAlive) {
            MyLOGD("android_main while engine.animating");

            drawFrame(&engine);
        }
    }

    gettimeofday(&timeTemp, NULL);

    appDeinit(&render);
    termDisplay(&engine);

    frameInfo.totalTime = (timeTemp.tv_usec / 1000000.0 + timeTemp.tv_sec) - frameInfo.totalTime;
    MyLOGD("totalTime=%f s, frameCount=%d, %.2f fps\n",
           frameInfo.totalTime, frameInfo.frameCount, frameInfo.frameCount / frameInfo.totalTime);

    MyLOGD("android_main end");
}