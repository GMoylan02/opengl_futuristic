#version 330 core

layout(location = 0) in vec3 position;

uniform mat4 MVP;

void main()
{
    // Transform the vertex position to light space
    gl_Position = MVP * vec4(position, 1.0);
}