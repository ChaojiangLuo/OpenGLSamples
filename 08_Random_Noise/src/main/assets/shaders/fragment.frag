#version 300 es

precision highp float;

in vec2 position;
uniform vec2 u_resolution;

in vec2 v_texCoord;
uniform sampler2D s_texture;

out vec4 fragColor;

float random (vec2 st) {
    return fract(sin(
    dot(st.xy, vec2(12.9898, 78.233)))*
    43758.5453123);
}

float rand1 (vec2 st) {
    return fract(sin(dot(st.xy, vec2(12.9898, 78.233)))*43758.5453123);
}

float rand2(vec2 n) {
    return 0.8 + 0.2 * fract(sin(dot(n.xy, vec2(12.9898, 78.233)))* 43758.5453);
}

float rand3 (in vec2 st) {
    vec2 i = floor(st);
    vec2 f = fract(st);

    // Four corners in 2D of a tile
    float a = random(i);
    float b = random(i + vec2(1.0, 0.0));
    float c = random(i + vec2(0.0, 1.0));
    float d = random(i + vec2(1.0, 1.0));

    vec2 u = smoothstep(0., 1., f);

    // Mix 4 coorners percentages
    return mix(a, b, u.x) + (c - a)* u.y * (1.0 - u.x) + (d - b) * u.x * u.y;
}

// Some useful functions
vec3 mod289(vec3 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec2 mod289(vec2 x) { return x - floor(x * (1.0 / 289.0)) * 289.0; }
vec3 permute(vec3 x) { return mod289(((x*34.0)+1.0)*x); }

float snoise4(vec2 v) {

    // Precompute values for skewed triangular grid
    const vec4 C = vec4(0.211324865405187,
    // (3.0-sqrt(3.0))/6.0
    0.366025403784439,
    // 0.5*(sqrt(3.0)-1.0)
    -0.577350269189626,
    // -1.0 + 2.0 * C.x
    0.024390243902439);
    // 1.0 / 41.0

    // First corner (x0)
    vec2 i  = floor(v + dot(v, C.yy));
    vec2 x0 = v - i + dot(i, C.xx);

    // Other two corners (x1, x2)
    vec2 i1 = vec2(0.0);
    i1 = (x0.x > x0.y)? vec2(1.0, 0.0):vec2(0.0, 1.0);
    vec2 x1 = x0.xy + C.xx - i1;
    vec2 x2 = x0.xy + C.zz;

    // Do some permutations to avoid
    // truncation effects in permutation
    i = mod289(i);
    vec3 p = permute(
    permute(i.y + vec3(0.0, i1.y, 1.0))
    + i.x + vec3(0.0, i1.x, 1.0));

    vec3 m = max(0.5 - vec3(
    dot(x0, x0),
    dot(x1, x1),
    dot(x2, x2)
    ), 0.0);

    m = m*m;
    m = m*m;

    vec3 x = 2.0 * fract(p * C.www) - 1.0;
    vec3 h = abs(x) - 0.5;
    vec3 ox = floor(x + 0.5);
    vec3 a0 = x - ox;

    // Normalise gradients implicitly by scaling m
    // Approximation of: m *= inversesqrt(a0*a0 + h*h);
    m *= 1.79284291400159 - 0.85373472095314 * (a0*a0+h*h);

    // Compute final noise value at P
    vec3 g = vec3(0.0);
    g.x  = a0.x  * x0.x  + h.x  * x0.y;
    g.yz = a0.yz * vec2(x1.x, x2.x) + h.yz * vec2(x1.y, x2.y);
    return 130.0 * dot(m, g);
}

void main() {
    vec4 colorNoise = vec4(0.);
    if (position.x < 0. && position.y > 0.) {
        colorNoise = vec4(rand1(position.xy), rand1(position.xy), rand1(position.xy), 1.0);
    } else if (position.x < 0. && position.y < 0.) {
        colorNoise = vec4(rand2(position.xy), rand2(position.xy), rand2(position.xy), 1.0);
    } else if (position.x > 0. && position.y < 0.) {
        vec2 st = gl_FragCoord.xy/u_resolution.xy;
        st.x *= u_resolution.x/u_resolution.y;
        vec3 color = vec3(0.0);

        // Scale the space in order to see the function
        st *= 10.;
        color = vec3(snoise4(st)*.5+.5);

        colorNoise = vec4(color, 1.0);
    } else if (position.x > 0. && position.y > 0.) {
        vec2 resolution = u_resolution / 64.0;

        vec2 st = gl_FragCoord.xy/resolution.xy;
        st.x *= resolution.x/resolution.y;
        vec3 color = vec3(0.0);

        // Scale the space in order to see the function
        st *= 10.;
        color = vec3(snoise4(st)*.5+.5);

        colorNoise = vec4(color, 1.0);
    }

    vec4 colorTexture = texture(s_texture, v_texCoord);

    if (colorTexture.r > 0. || colorTexture.g > 0. || colorTexture.b > 0.) {
        fragColor =  colorTexture;
    } else {
        fragColor = colorNoise;
    }
    //fragColor = colorNoise+colorTexture-(2.0*colorNoise*colorTexture);

    //fragColor = colorNoise/(vec4(1.0)-colorTexture);

    //fragColor = vec4(1.0)-(vec4(1.0)-colorNoise)/colorTexture;

    //fragColor = abs(colorNoise-colorTexture);
}