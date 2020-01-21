#version 300 es

uniform float u_offset;
layout(location = 0) in vec4 a_position;
layout(location = 1) in vec2 a_texCoord;
out vec2 v_texCoord;

// 高斯算子左右偏移值，当偏移值为5时，高斯算子为 11 x 11
const int SHIFT_SIZE = 5;
out vec4 blurShiftCoordinates[SHIFT_SIZE];

uniform highp float texelWidthOffset;
uniform highp float texelHeightOffset;

void main() {
    gl_Position = a_position;
    gl_Position.x += u_offset;
    v_texCoord = a_texCoord;

    vec2 singleStepOffset = vec2(texelWidthOffset, texelHeightOffset);

    // 记录偏移坐标
    for (int i = 0; i < SHIFT_SIZE; i++) {
        blurShiftCoordinates[i] = vec4(a_texCoord - float(i + 1) * singleStepOffset, a_texCoord + float(i + 1) * singleStepOffset);
    }
}