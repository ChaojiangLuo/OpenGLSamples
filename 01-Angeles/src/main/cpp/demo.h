/* San Angeles Observation OpenGL ES version example
 * Copyright 2004-2005 Jetro Lauha
 * All rights reserved.
 * Web: http://iki.fi/jetro/
 *
 * This source is free software; you can redistribute it and/or
 * modify it under the terms of EITHER:
 *   (1) The GNU Lesser General Public License as published by the Free
 *       Software Foundation; either version 2.1 of the License, or (at
 *       your option) any later version. The text of the GNU Lesser
 *       General Public License is included with this source in the
 *       file LICENSE-LGPL.txt.
 *   (2) The BSD-style license that is included with this source in
 *       the file LICENSE-BSD.txt.
 *
 * This source is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE. See the files
 * LICENSE-LGPL.txt and LICENSE-BSD.txt for more details.
 *
 * $Id: app.h,v 1.14 2005/02/06 21:13:54 tonic Exp $
 * $Revision: 1.14 $
 */

#ifndef APP_H_INCLUDED
#define APP_H_INCLUDED

#include <GLES/gl.h>

#include "shapes.h"

#ifdef __cplusplus
extern "C" {
#endif

// Total run length is 20 * camera track base unit length (see cams.h).
#define RUN_LENGTH  (20 * CAMTRACK_LEN)
#undef PI
#define PI 3.1415926535897932f
#define RANDOM_UINT_MAX 65535

#define WINDOW_DEFAULT_WIDTH    640
#define WINDOW_DEFAULT_HEIGHT   480

#define WINDOW_BPP              16

// Definition of one GL object in this demo.
typedef struct {
    /* Vertex array and color array are enabled for all objects, so their
     * pointers must always be valid and non-NULL. Normal array is not
     * used by the ground plane, so when its pointer is NULL then normal
     * array usage is disabled.
     *
     * Vertex array is supposed to use GL_FIXED datatype and stride 0
     * (i.e. tightly packed array). Color array is supposed to have 4
     * components per color with GL_UNSIGNED_BYTE datatype and stride 0.
     * Normal array is supposed to use GL_FIXED datatype and stride 0.
     */
    GLfixed *vertexArray;
    GLubyte *colorArray;
    GLfixed *normalArray;
    GLint vertexComponents;
    GLsizei count;
} GLOBJECT;

/**
 * App saved state data.
 */
struct OpenGLRender {
    int isAlive;
    long sStartTick;
    long sTick;

    int sCurrentCamTrack;
    long sCurrentCamTrackStartTick;
    long sNextCamTrackStartTick;

    GLOBJECT *sSuperShapeObjects[SUPERSHAPE_COUNT];
    GLOBJECT *sGroundPlane;
};

// The simple framework expects the application code to define these functions.
extern void appInit(struct OpenGLRender* render);
extern void appDeinit(struct OpenGLRender* render);
extern void appRender(struct OpenGLRender* render, long tick, int width, int height);

#ifdef __cplusplus
}
#endif


#endif // !APP_H_INCLUDED
