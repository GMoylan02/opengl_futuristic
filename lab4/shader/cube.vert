#version 330 core

// Input
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexUV;

// Output data, to be interpolated for each fragment
const int MAXLIGHTS = 10;

out vec3 worldPosition;
out vec3 worldNormal;
out vec4 FragPosLightSpaces[MAXLIGHTS];
out vec2 uv;

// Matrix for vertex transformation
uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;
uniform mat4 lightSpaceMatrices[MAXLIGHTS];

void main() {
    // Transform vertex
    //gl_Position =  MVP * vec4(vertexPosition, 1);
    gl_Position = /*projection * */view * model * vec4(vertexPosition, 1.0);

    uv = vertexUV;

    worldPosition = vec3(model * vec4(vertexPosition, 1.0));
    worldNormal = mat3(transpose(inverse(model))) * vertexNormal;
    // Calculate light-space positions
    for (int i = 0; i < MAXLIGHTS; ++i) {
        FragPosLightSpaces[i] = lightSpaceMatrices[i] * vec4(worldPosition, 1.0);
    }
}
