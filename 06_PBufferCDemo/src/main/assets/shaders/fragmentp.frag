#version 300 es

precision mediump float;

uniform vec4 uuColor;

out vec4 fragColor;

void main() {
    fragColor = uuColor;
}