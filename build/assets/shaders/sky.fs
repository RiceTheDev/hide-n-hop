#version 330

in vec3 vRayDir;

uniform vec3 lightDir;
uniform vec4 lightColor;
uniform vec4 skyColor;
uniform vec4 horizonColor;

out vec4 finalColor;

void main()
{
    vec3 rayDir = normalize(vRayDir);
    vec3 sunDir = normalize(-lightDir);

    float height = clamp(rayDir.y, 0.0, 1.0);
    vec3 skyGrad = mix(horizonColor.rgb, skyColor.rgb, pow(height, 0.7));

    if (rayDir.y < 0.0)
    {
        skyGrad = mix(horizonColor.rgb, horizonColor.rgb * 0.4, clamp(-rayDir.y * 3.0, 0.0, 1.0));
    }

    float sunDot = max(dot(rayDir, sunDir), 0.0);

    float sunDisk = smoothstep(0.9985, 0.9996, sunDot);

    float sunHalo = pow(sunDot, 16.0) * 0.45;
    float sunGlow = pow(sunDot, 64.0) * 0.85;

    vec3 sunLight = (lightColor.rgb * (sunHalo + sunGlow)) + (vec3(1.0) * sunDisk * 3.0);
    vec3 finalRGB = skyGrad + sunLight;

    finalColor = vec4(finalRGB, 1.0);
}
