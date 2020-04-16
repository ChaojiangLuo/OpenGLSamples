#version 300 es

uniform vec2 v_offset;

layout(location = 0) in vec4 vPosition;
layout(location = 1) in vec2 ttexCoord;

out vec2 position;
out vec2 v_texCoord;

void main() {
    v_texCoord = ttexCoord;

    position = vPosition.xy;
    gl_Position.xy = position;
    gl_Position.zw = vPosition.zw;
}