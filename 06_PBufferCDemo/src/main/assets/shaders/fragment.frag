#version 300 es

precision mediump float;

uniform vec4 uuColor;

uniform bool isFragTexture;

in vec2 t_texCoord;
uniform sampler2D s_texture;

out vec4 fragColor;

void main() {
    if (isFragTexture) {
        fragColor = texture(s_texture, t_texCoord);
    } else {
        fragColor = uuColor;
    }
}