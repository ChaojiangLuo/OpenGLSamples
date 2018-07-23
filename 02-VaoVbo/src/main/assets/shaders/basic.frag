#version 320 es
precision mediump float;

in vec3 Color;
out vec4 FragColor;

void main() {
    FragColor = vec4(Color,0.6);
}

