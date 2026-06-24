#version 330

in vec2 fragTexCoord;
out vec4 fragColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

const float SCANLINES = 144.0;
const float PI = 3.14159;

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

    vec4 baseColor = vec4(r, g, b, 1.0);

    // Blur 7x7
    float off = 0.004;
    vec4 blur = vec4(0.0);
    for (int y = -3; y <= 3; y++)
        for (int x = -3; x <= 3; x++)
            blur += texture(texture0, uv + vec2(float(x) * off, float(y) * off));
    blur /= 49.0;

    // Glow: excess del blur sobre el original
    vec4 excess = max(blur - baseColor, 0.0);
    float avgExcess = (excess.r + excess.g + excess.b) / 3.0;
    float glowAmount = smoothstep(0.005, 0.04, avgExcess) * 0.25;

    vec4 color = baseColor + excess * glowAmount;

    // Scanlines
    float scanline = abs(sin(uv.y * SCANLINES * PI));
    float scanIntensity = 1.0 - scanline * 0.15;

    fragColor = color * scanIntensity * colDiffuse;
}
