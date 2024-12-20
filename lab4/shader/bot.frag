#version 330 core

in vec3 color;
//in vec2 uv;

// TODO: To add UV input to this fragment shader

// TODO: To add the texture sampler
//uniform sampler2D textureSampler;

out vec4 finalColor;

void main()
{
	// TODO: fix
	finalColor = vec4(color,1);
	//finalColor = texture(textureSampler, uv);
	// TODO: texture lookup.

}
