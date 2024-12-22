#version 330 core

// Input
layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexUV;

// Output data, to be interpolated for each fragment
out vec3 worldPosition;
out vec3 worldNormal;
out vec4 FragPosLightSpace;
out vec2 uv;

uniform mat4 model;
uniform mat4 view;
uniform mat4 projection;


void main() {
    // Transform vertex
    //gl_Position =  MVP * vec4(vertexPosition, 1);
    gl_Position = /*projection * */view * model * vec4(vertexPosition, 1.0);

    // Pass vertex color to the fragment shader
    //color = vertexColor;

    uv = vertexUV;

    worldPosition = vec3(model * vec4(vertexPosition, 1.0));
    worldNormal = mat3(transpose(inverse(model))) * vertexNormal;
    FragPosLightSpace = model * vec4(vertexPosition, 1.0);
}



/*
void main() {
    gl_Position =  view * model * vec4(vertexPosition, 1);
}

*/