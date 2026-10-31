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

    if (projCoords.z > 1.0 || projCoords.z < 0.0 ||
        projCoords.x < 0.0 || projCoords.x > 1.0 ||
        projCoords.y < 0.0 || projCoords.y > 1.0)
    {
        return 0.0;
    }

    float ndotl = dot(normal, lightD);
    if (ndotl <= 0.0)
        return 1.0;

    float bias = max(0.0025 * (1.0 - ndotl), 0.0006);
    vec2 texelSize = 1.0 / vec2(textureSize(shadowMap, 0));
    float shadow = 0.0;

    for (int x = -1; x <= 1; x++)
    {
        for (int y = -1; y <= 1; y++)
        {
            float pcfDepth = texture(shadowMap, projCoords.xy + vec2(x, y) * texelSize).r;
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
    vec3 ambient = mix(groundColor.rgb, skyColor.rgb, hemiFactor) * 0.40;

    float ndotl = dot(normal, lightD);
    float directWrap = max((ndotl + 0.12) / 1.12, 0.0);
    vec3 diffuse = lightColor.rgb * directWrap * 0.90;

    float fresnel = pow(1.0 - max(dot(normal, viewD), 0.0), 3.5);
    vec3 morningGlow = lightColor.rgb * fresnel * max(dot(lightD, -viewD), 0.0) * 0.28;

    vec3 halfDir = normalize(lightD + viewD);
    float specAngle = max(dot(normal, halfDir), 0.0);
    float specularStrength = pow(specAngle, 48.0) * 0.35;
    vec3 specular = lightColor.rgb * specularStrength;

    float shadow = ShadowCalculation(fragPosLightSpace, normal, lightD);
    vec3 directLight = (1.0 - shadow) * (diffuse + specular);

    vec3 litColor = (ambient + directLight + morningGlow) * texelColor.rgb;

    float dist = length(viewPos - fragPosition);
    float fogFactor = exp(-pow(dist * fogDensity, 1.7));
    fogFactor = clamp(fogFactor, 0.0, 1.0);

    vec3 finalRGB = mix(fogColor.rgb, litColor, fogFactor);

    finalRGB = pow(max(finalRGB, vec3(0.0)), vec3(1.0 / 1.02));

    finalColor = vec4(finalRGB, texelColor.a);
}
