#version 330

in vec3 fragPosition;
in vec2 fragTexCoord;
in vec4 fragColor;
in vec3 fragNormal;
in vec4 fragPosLightSpace;

uniform sampler2D texture0;
uniform sampler2D shadowMap;
uniform vec4 colDiffuse;

uniform vec3 viewPos;
uniform vec3 lightDir;
uniform vec4 lightColor;
uniform vec4 skyColor;
uniform vec4 groundColor;
uniform vec4 fogColor;
uniform float fogDensity;

out vec4 finalColor;

float ShadowCalculation(vec4 posLightSpace, vec3 normal, vec3 lightD)
{
    if (posLightSpace.w <= 0.0)
        return 0.0;

    vec3 projCoords = posLightSpace.xyz / posLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5;

    float ndotl = abs(dot(normal, lightD));

    if (projCoords.z > 1.0)
        return 1.0;
    if (projCoords.x < 0.01 || projCoords.x > 0.99 ||
        projCoords.y < 0.01 || projCoords.y > 0.99 ||
        projCoords.z < 0.0)
        return 0.0;

    float bias = max(0.0035 * (1.0 - ndotl), 0.001);
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));
    float shadow = 0.0;

    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            vec2 sampleUV = clamp(projCoords.xy + vec2(x, y) * texelSize, 0.0, 1.0);
            float pcfDepth = texture(shadowMap, sampleUV).r;
            shadow += (projCoords.z - bias > pcfDepth) ? 1.0 : 0.0;
        }
    }

    return shadow / 9.0;
}

void main()
{
    vec4 texelColor = texture(texture0, fragTexCoord) * colDiffuse * fragColor;

    if (texelColor.a < 0.1)
        discard;

    vec3 normal = normalize(fragNormal);
    vec3 lightD = normalize(-lightDir);
    vec3 viewD  = normalize(viewPos - fragPosition);

    float hemiFactor = normal.y * 0.5 + 0.5;
    vec3 ambient = mix(groundColor.rgb, skyColor.rgb, hemiFactor) * 0.38;

    float ndotl = max(dot(normal, lightD), 0.0);
    vec3 diffuse = lightColor.rgb * ndotl * 0.90;

    vec3 halfDir = normalize(lightD + viewD);
    float specAngle = max(dot(normal, halfDir), 0.0);
    float specFactor = (ndotl > 0.0) ? pow(specAngle, 32.0) * 0.25 : 0.0;
    vec3 specular = lightColor.rgb * specFactor;

    float fresnel = pow(1.0 - max(dot(normal, viewD), 0.0), 3.0);
    float rimFactor = (ndotl > 0.0) ? fresnel * max(dot(lightD, -viewD), 0.0) * 0.20 : 0.0;
    vec3 rimGlow = lightColor.rgb * rimFactor;

    float shadow = ShadowCalculation(fragPosLightSpace, normal, lightD);

    vec3 direct = (1.0 - shadow) * (diffuse + specular + rimGlow);
    vec3 litColor = (ambient + direct) * texelColor.rgb;

    float dist = length(viewPos - fragPosition);
    float fogFactor = exp(-pow(dist * fogDensity, 1.7));
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    vec3 finalRGB = mix(fogColor.rgb, litColor, fogFactor);

    finalColor = vec4(finalRGB, texelColor.a);
}
