#version 330 core

const int MAXLIGHTS = 10;      // Maximum number of lights

in vec3 worldPosition;         // World-space position of the fragment
in vec3 worldNormal;           // World-space normal of the fragment
in vec2 uv;                    // Texture coordinates
in vec4 FragPosLightSpaces[MAXLIGHTS];

const float gamma = 2.2;       // Gamma correction factor
const float exposure = 1.0;    // Exposure for tone mapping

// Texture and lighting uniforms
uniform sampler2D textureSampler;      // Object texture
uniform int numLights;                 // Number of active lights
uniform vec3 lightPositions[MAXLIGHTS];// Positions of all lights
uniform vec3 lightIntensities[MAXLIGHTS];// Intensities of all lights
uniform vec3 viewPos;                  // Camera position (for specular calculation)
uniform sampler2D shadowMaps[MAXLIGHTS];

// Output color
out vec4 finalColor;

// Function to calculate lighting
vec3 calculateLighting(vec3 normal);
float calculateShadow(int lightIndex, vec4 fragPosLightSpace, float bias);

void main()
{
    // Normalize the normal vector
    vec3 normal = normalize(worldNormal);

    // Calculate the lighting contribution
    vec3 lighting = calculateLighting(normal);

    // Output the final color
    finalColor = vec4(lighting, 1.0);
    //finalColor = vec4(1,0,0,1);
}

vec3 calculateLighting(vec3 normal) {
    vec4 sampledTexture = texture(textureSampler, uv);
    vec3 objectColor = sampledTexture.rgb;

    vec3 ambientColor = vec3(0.2, 0.2, 0.2) * objectColor;
    vec3 result = ambientColor;

    for (int i = 0; i < min(numLights, MAXLIGHTS); ++i) {
        vec3 lightDir = normalize(lightPositions[i] - worldPosition);
        float diff = max(dot(normal, lightDir), 0.0);
        //if (diff <= 0.0)
        //    return vec3(1.0); // No shadow on back-facing sides
        vec3 diffuse = diff * lightIntensities[i] * objectColor;

        float specularStrength = 0.5;
        vec3 viewDir = normalize(viewPos - worldPosition);
        vec3 reflectDir = reflect(-lightDir, normal);
        float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32);
        vec3 specular = specularStrength * spec * lightIntensities[i];

        // Calculate shadow factor
        float bias = max(0.005 * (1.0 - dot(normalize(worldNormal), normalize(lightPositions[i] - worldPosition))), 0.005);
        float shadow = calculateShadow(i, FragPosLightSpaces[i], bias);
        vec3 lightContribution = (diffuse + specular) * shadow;

        result += lightContribution;
        //result += shadow;
    }

    return result;
}

float calculateShadow(int lightIndex, vec4 fragPosLightSpace, float bias) {
    // Perform perspective divide
    vec3 projCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
    projCoords = projCoords * 0.5 + 0.5; // Transform to [0, 1]

    if (projCoords.x < 0.0 || projCoords.x > 1.0 || projCoords.y < 0.0 || projCoords.y > 1.0 || fragPosLightSpace.w <= 0.0)
        return 1.0; // Outside the shadow map; assume no shadow


    // Get depth from shadow map
    float closestDepth = texture(shadowMaps[lightIndex], projCoords.xy).r;
    float currentDepth = projCoords.z;

    float shadow = (currentDepth - bias > closestDepth) ? 0.0 : 1.0;

    // Return shadow factor
    return shadow;
}
