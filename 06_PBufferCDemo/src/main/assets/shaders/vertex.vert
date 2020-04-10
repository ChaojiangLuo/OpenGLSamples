#version 300 es

uniform bool blur;

layout(location = 0) in vec4 vvPosition;
layout(location = 1) in vec4 tvPosition;
layout(location = 2) in vec2 a_texCoord;

uniform bool isTexture;

out vec2 t_texCoord;

void main() {
    if (isTexture) {
        gl_Position = tvPosition;
        t_texCoord = a_texCoord;
    } else {
        gl_Position = vvPosition;
    }
}