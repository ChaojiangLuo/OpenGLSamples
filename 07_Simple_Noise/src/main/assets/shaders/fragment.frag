#version 300 es

precision mediump float;

in vec2 v_texCoord;
layout(location = 0) out vec4 outColor;

uniform sampler2D s_texture;
uniform sampler2D s_RandTex;

const mat3 transform = mat3(
vec3(0.794750269, 0.011856037, 0.001236832),
vec3(-0.00046911, 0.783690389, 0.0000112739),
vec3(-0.000163865, -0.001859539, 0.745181949));

void main()
{
    outColor.xyz = transform * texture(s_texture, v_texCoord).xyz;

    outColor.x = outColor.x + texture(s_RandTex, v_texCoord).x;
    outColor.y = outColor.y + texture(s_RandTex, v_texCoord).x;
    outColor.z = outColor.z + texture(s_RandTex, v_texCoord).x;
    outColor.w = 1.0;
}