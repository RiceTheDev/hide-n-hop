#version 330

in vec3 fragPosition;
out vec4 finalColor;

uniform float progress;

vec3 hash33(vec3 p) {
    p = vec3(dot(p, vec3(127.1, 311.7, 74.7)),
             dot(p, vec3(269.5, 183.3, 246.1)),
             dot(p, vec3(113.5, 271.9, 124.6)));
    return -1.0 + 2.0 * fract(sin(p) * 43758.5453123);
}

float gnoise(vec3 p) {
    vec3 i = floor(p);
    vec3 f = fract(p);
    vec3 u = f * f * (3.0 - 2.0 * f);
    return mix(
        mix(mix(dot(hash33(i + vec3(0,0,0)), f - vec3(0,0,0)),
                dot(hash33(i + vec3(1,0,0)), f - vec3(1,0,0)), u.x),
            mix(dot(hash33(i + vec3(0,1,0)), f - vec3(0,1,0)),
                dot(hash33(i + vec3(1,1,0)), f - vec3(1,1,0)), u.x), u.y),
        mix(mix(dot(hash33(i + vec3(0,0,1)), f - vec3(0,0,1)),
                dot(hash33(i + vec3(1,0,1)), f - vec3(1,0,1)), u.x),
            mix(dot(hash33(i + vec3(0,1,1)), f - vec3(0,1,1)),
                dot(hash33(i + vec3(1,1,1)), f - vec3(1,1,1)), u.x), u.y), u.z);
}

float ridged(vec3 p) {
    float v = 0.0, amp = 0.5, freq = 1.0;
    for (int i = 0; i < 4; i++) {
        float n = 1.0 - abs(gnoise(p * freq));
        n = n * n;
        v += amp * n;
        freq *= 2.17;
        amp *= 0.5;
        p.y -= 0.40;
    }
    return v;
}

float smoothn(vec3 p) {
    float v = 0.0, amp = 0.5, freq = 1.0;
    for (int i = 0; i < 4; i++) {
        v += amp * gnoise(p * freq);
        freq *= 2.03;
        amp *= 0.5;
        p.y -= 0.35;
    }
    return v;
}

float posterize(float x, float bands) {
    float s = x * bands;
    float b = floor(s);
    return (b + smoothstep(0.7, 1.0, fract(s))) / bands;
}

void main() {
    // Direção no "unit sphere"
    vec3 dir = normalize(fragPosition);

    vec3 pos = dir * 3.0;
    pos.y  -= progress * 7.0;
    pos.xz += dir.xz * progress * 1.5;
    pos   *= 1.0 + progress * 0.8;

    vec3 warp = vec3(
        smoothn(pos + vec3(1.7, 9.2, 4.1)),
        smoothn(pos + vec3(8.3, 2.8, 5.5)),
        smoothn(pos + vec3(3.1, 6.4, 1.2))
    ) * 0.55;

    float fire = ridged(pos * 1.4 + warp * 2.2);
    float turb = smoothn(pos * 2.2 + warp * 1.5);
    float n = clamp(fire * 0.75 + turb * 0.25 + 0.15, 0.0, 1.0);

    float heat = 1.0 - progress * 0.75;

    float ember = smoothn(pos * 6.0 + vec3(progress * 5.0, -progress * 8.0, progress * 3.0));
    ember = smoothstep(0.30, 0.55, ember) * heat;

    float intensity = n * (0.9 + heat * 1.0) + ember * 0.5;

    float dissolveBias = mix(0.30, 0.80, progress);
    if (intensity < dissolveBias) discard;

    float alpha = smoothstep(dissolveBias, dissolveBias + 0.15, intensity);
    alpha *= 1.0 - smoothstep(0.85, 1.0, progress);
    if (alpha < 0.02) discard;

    float t = clamp(intensity, 0.0, 1.0);
    t = posterize(t, 5.0);

    vec3 cSmoke  = vec3(0.10, 0.10, 0.12);
    vec3 cDark   = vec3(0.45, 0.08, 0.02);
    vec3 cRed    = vec3(0.90, 0.22, 0.03);
    vec3 cOrange = vec3(1.00, 0.48, 0.06);
    vec3 cYellow = vec3(1.00, 0.82, 0.30);
    vec3 cWhite  = vec3(1.00, 0.98, 0.92);

    vec3 color;
    if      (t < 0.30) color = mix(cSmoke,  cDark,   t / 0.30);
    else if (t < 0.50) color = mix(cDark,   cRed,    (t - 0.30) / 0.20);
    else if (t < 0.68) color = mix(cRed,    cOrange, (t - 0.50) / 0.18);
    else if (t < 0.85) color = mix(cOrange, cYellow, (t - 0.68) / 0.17);
    else               color = mix(cYellow, cWhite,  (t - 0.85) / 0.15);

    color = mix(color, vec3(1.0, 0.92, 0.55), ember * 1.5);

    float smokeT = smoothstep(0.60, 1.0, progress);
    vec3 smokeColor = vec3(0.14, 0.13, 0.13) * (0.5 + n * 0.5);
    color = mix(color, smokeColor, smokeT * 0.7);

    color *= 1.0 + heat * 1.2;

    finalColor = vec4(color, alpha);
}
