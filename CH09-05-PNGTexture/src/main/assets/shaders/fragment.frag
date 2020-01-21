#version 300 es

precision mediump float;
in vec2 v_texCoord;
layout(location = 0) out vec4 outColor;
uniform sampler2D s_texture;

uniform bool blur;
uniform bool isVertical;

void main() {
    if (!blur) {
        outColor = texture(s_texture, v_texCoord);
    } else {
        vec2 tex_offset =vec2(1.0/300.0, 1.0/300.0);
        vec4 orColor=texture(s_texture, v_texCoord);
        float orAlpha=orColor.a;
        float weight[5] = float[] (0.227027, 0.1945946, 0.1216216, 0.054054, 0.016216);
        vec3 color=orColor.rgb*weight[0];
        if (!isVertical)
        {
            for (int i=1;i<5;i++)
            {
                color+=texture(s_texture, v_texCoord+vec2(tex_offset.x * float(i), 0.0)).rgb*weight[i];
                color+=texture(s_texture, v_texCoord-vec2(tex_offset.x * float(i), 0.0)).rgb*weight[i];

            }
        }
        else
        {
            for (int i=1;i<5;i++)
            {
                color+=texture(s_texture, v_texCoord+vec2(0.0, tex_offset.y * float(i))).rgb*weight[i];
                color+=texture(s_texture, v_texCoord-vec2(0.0, tex_offset.y * float(i))).rgb*weight[i];
            }
        }
        outColor=vec4(color, orAlpha);
    }
}