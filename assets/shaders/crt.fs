#version 330

in vec2 fragTexCoord;
out vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

const float SCANLINES = 144.0;

void main() {
    vec2 uv = fragTexCoord;

    // Barrel distortion
    float dist = 0.08;
    vec2 cent = uv - 0.5;
    float r2 = dot(cent, cent);
    uv = uv + cent * r2 * dist;
    uv = clamp(uv, 0.0, 1.0);

    // Chromatic aberration
    float ca = 0.002;
    float r = texture(texture0, uv + vec2(ca, 0.0)).r;
    float g = texture(texture0, uv).g;
    float b = texture(texture0, uv - vec2(ca, 0.0)).b;

    // Scanlines (una por fila de pixel del juego)
    float scanline = abs(sin(uv.y * SCANLINES * 3.14159));
    float scanIntensity = 1.0 - scanline * 0.2;

    vec4 color = vec4(r, g, b, 1.0) * scanIntensity * colDiffuse;
    fragColor = color;
}
