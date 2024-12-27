#version 330 core

// Input
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexUV;
layout(location = 3) in mat4 instance;

// Output data, to be interpolated for each fragment
out vec3 worldPosition;
out vec3 worldNormal;
out vec2 uv;
out mat4 model;


// Matrices for vertex transformation
uniform mat4 camera;

void main() {
    // Transform vertex
    gl_Position = camera * instance * vec4(vertexPosition, 1);

    // World-space geometry
    worldPosition = vertexPosition;
    worldNormal = vertexNormal;

    // Pass UV to the fragment shader
    uv = vertexUV;
    model = instance;
}