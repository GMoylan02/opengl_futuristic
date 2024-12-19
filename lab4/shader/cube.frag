#version 330 core

in vec3 color;
in vec3 worldPosition;
in vec3 worldNormal;
in vec2 uv;
in vec4 FragPosLightSpace; // Position in light space

const int MAXLIGHTS = 10;
const float gamma = 2.2;
const float exposure = 1.0;

uniform sampler2D textureSampler;
uniform int numLights;            // Number of active lights
uniform vec3 lightPositions[MAXLIGHTS];
uniform vec3 lightIntensities[MAXLIGHTS];
uniform vec3 ambientColor; // Default to a dim color, e.g., vec3(0.2, 0.2, 0.2)

out vec4 finalColor;

vec3 calculateLighting(vec3 normal);

void main()
{
    vec3 normal = normalize(worldNormal);
    vec3 lighting = calculateLighting(normal);
    finalColor = vec4(lighting, 1); // debug
}

vec3 calculateLighting(vec3 normal) {
    vec4 sampledTexture = texture(textureSampler, uv);
    vec3 result = ambientColor;
    for (int i = 0; i < min(numLights, MAXLIGHTS); ++i) {
        vec3 lightDir = normalize(lightPositions[i] - worldPosition);
        float diff = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = diff * lightIntensities[i] * sampledTexture.rgb;
        vec3 exposedColor = diffuse * exposure;
        vec3 toneMappedColor = exposedColor / (exposedColor + vec3(1.0));
        result += pow(toneMappedColor, vec3(1.0 / 2.2));
    }
    return result;
}
