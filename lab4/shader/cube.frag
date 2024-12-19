#version 330 core

in vec3 color;
in vec3 worldPosition;
in vec3 worldNormal;
in vec2 uv;

in vec4 FragPosLightSpace; // Position in light space

// TODO: To add UV input to this fragment shader

// TODO: To add the texture sampler
uniform sampler2D textureSampler;
uniform int numLights;            // Number of active lights
const int max = 10;
uniform vec3 lightPositions[max];
uniform vec3 lightIntensities[max];


const float gamma = 2.2;

out vec4 finalColor;

vec3 calculateLighting(vec3 normal);

void main()
{
	//finalColor = color;
    vec4 sampledTexture = texture(textureSampler, uv);
    vec3 normal = normalize(worldNormal);
    vec3 lighting = calculateLighting(normal);
    vec4 textAndLight = sampledTexture * vec4(lighting, 1.0);
    vec3 gammaCorrected = pow(textAndLight.rgb, vec3(1.0 / gamma));
    finalColor = vec4(gammaCorrected, textAndLight.a);
}

vec3 calculateLighting(vec3 normal) {
    vec3 result = vec3(0.0);
    for (int i = 0; i < numLights; ++i) {
        vec3 lightDir = normalize(lightPositions[i] - worldPosition);
        float diff = max(dot(normal, lightDir), 0.0);
        vec3 diffuse = diff * lightIntensities[i] * color;
        vec3 exposedColor = diffuse * exposure;
        vec3 toneMappedColor = exposedColor / (exposedColor + vec3(1.0));
        result += toneMappedColor * vec3(1.0);
    }
    return result;
}
