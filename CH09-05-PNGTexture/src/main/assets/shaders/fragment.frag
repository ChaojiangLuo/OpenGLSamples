#version 300 es

precision mediump float;
in vec2 v_texCoord;
layout(location = 0) out vec4 outColor;
uniform sampler2D s_texture;

uniform bool blur;
const int SHIFT_SIZE = 5; // 高斯算子左右偏移值
in vec4 blurShiftCoordinates[SHIFT_SIZE];

void main() {
    if (!blur) {
        outColor = texture(s_texture, v_texCoord);
    } else {
        // 计算当前坐标的颜色值
        vec4 currentColor = texture(s_texture, v_texCoord);
        vec3 sum = currentColor.rgb;
        // 计算偏移坐标的颜色值总和
        for (int i = 0; i < SHIFT_SIZE; i++) {
            sum += texture(s_texture, blurShiftCoordinates[i].xy).rgb;
            sum += texture(s_texture, blurShiftCoordinates[i].zw).rgb;
        }
        outColor = vec4(sum / float(2 * SHIFT_SIZE + 1), currentColor.a);
    }
}