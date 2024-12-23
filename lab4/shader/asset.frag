#version 330 core

in vec3 worldPosition;         // World-space position of the fragment
in vec3 worldNormal;           // World-space normal of the fragment
in vec2 uv;                    // Texture coordinates
in vec4 FragPosLightSpace;     // Position in light space (optional for shadows)

const int MAXLIGHTS = 10;      // Maximum number of lights
const float gamma = 2.2;       // Gamma correction factor
const float exposure = 1.0;    // Exposure for tone mapping

// Texture and lighting uniforms
uniform sampler2D textureSampler;      // Object texture
uniform int numLights;                 // Number of active lights
uniform vec3 lightPositions[MAXLIGHTS];// Positions of all lights
uniform vec3 lightIntensities[MAXLIGHTS];// Intensities of all lights
uniform vec3 viewPos;                  // Camera position (for specular calculation)

// Output color
out vec4 finalColor;

// Function to calculate lighting
vec3 calculateLighting(vec3 normal);

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
	// Sample the texture to get the base object color
	vec4 sampledTexture = texture(textureSampler, uv);
	//vec4 sampledTexture = vec4(1,1,1,1);
	vec3 objectColor = sampledTexture.rgb;

	// Ambient lighting
	vec3 ambientColor = vec3(0.2, 0.2, 0.2) * objectColor;

	// Initialize the result with the ambient contribution
	vec3 result = ambientColor;

	// Iterate over active lights
	for (int i = 0; i < min(numLights, MAXLIGHTS); ++i) {
		// Direction from the fragment to the light
		vec3 lightDir = normalize(lightPositions[i] - worldPosition);

		// Diffuse lighting (Lambertian reflectance)
		float diff = max(dot(normal, lightDir), 0.0);
		vec3 diffuse = diff * lightIntensities[i] * objectColor;

		// Specular lighting (Blinn-Phong model)
		float specularStrength = 0.5;
		vec3 viewDir = normalize(viewPos - worldPosition);
		vec3 reflectDir = reflect(-lightDir, normal);
		float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32); // Shininess factor = 32
		vec3 specular = specularStrength * spec * lightIntensities[i];

		// Combine diffuse and specular components
		vec3 lightContribution = diffuse + specular;

		// Apply exposure and tone mapping
		vec3 exposedColor = lightContribution * exposure;
		vec3 toneMappedColor = exposedColor / (exposedColor + vec3(1.0));

		// Add the tone-mapped color contribution
		result += pow(toneMappedColor, vec3(1.0 / gamma));
	}

	return result;
}


/*
void main() {
    finalColor = vec4(1,0,0,1);
}

*/
