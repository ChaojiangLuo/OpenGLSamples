#version 300 es

layout(location = 0) in vec4 vvPosition;

void main() {
    gl_Position = vvPosition;
}