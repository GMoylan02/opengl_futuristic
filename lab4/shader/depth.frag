#version 330 core

void main()
{
    // No colour needed, only depth
    gl_FragDepth = pow(gl_FragCoord.z, 500);
}