//
// Created by luocj on 18-7-13.
//

#ifndef OPENGLSAMPLES_NATIVE_MAIN_H
#define OPENGLSAMPLES_NATIVE_MAIN_H

#include "demo.h"

/**
 * Same as Media Player
 */
enum Play_State {
    STATE_IDLE = 0,
    STATE_SET_DATASOURCE_PENDING,
    STATE_UNPREPARED,
    STATE_PREPARING,
    STATE_PREPARED,
    STATE_RUNNING,
    STATE_PAUSED,
    STATE_RESET_IN_PROGRESS,
    STATE_STOPPED,                  // equivalent to PAUSED
    STATE_STOPPED_AND_PREPARING,    // equivalent to PAUSED, but seeking
    STATE_STOPPED_AND_PREPARED,     // equivalent to PAUSED, but seek complete
};

/**
 * App saved state data.
 */
struct Saved_State {
    Play_State state;
};

struct FrameInfo {
    int frameCount;
    double totalTime;
};

/**
 * Shared state for our app.
 */
struct Engine {
    struct android_app *app;;

    ASensorManager *sensorManager;
    const ASensor *accelerometerSensor;
    ASensorEventQueue *sensorEventQueue;

    EGLint samples;
    EGLDisplay display;
    EGLSurface surface;
    EGLContext context;
    int32_t width;
    int32_t height;

    int isAlive;
    Play_State playState;
    Saved_State savedState;

    struct OpenGLRender* render;
    FrameInfo frameInfo;
};

#endif //OPENGLSAMPLES_NATIVE_MAIN_H
