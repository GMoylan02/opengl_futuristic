#version 330 core

in vec3 worldPosition;
in vec3 worldNormal;
in vec2 uv;
in mat4 model;

const int MAX_LIGHTS = 100;

uniform sampler2D textureSampler;
uniform vec4 baseColorFactor;
uniform int isLight;

out vec4 finalColor;

uniform int lightCount;
uniform vec3 lightPositions[MAX_LIGHTS];
uniform vec3 lightIntensities[MAX_LIGHTS];
uniform float lightExposures[MAX_LIGHTS];
uniform sampler2DArray shadowMapArray;
uniform mat4 lightSpaceMatrices[MAX_LIGHTS];
uniform vec3 cameraPosition;

void main()
{
	vec3 normal = normalize(worldNormal);
	vec3 fragPosition = vec3(model * vec4(worldPosition, 1.0));
	vec3 res = vec3(0.0);
	for (int i = 0; i < lightCount; i++) {
		vec3 lightDirection = normalize(lightPositions[i] - fragPosition);
		float distance = length(lightPositions[i] - fragPosition);

		float attenuation = 1.0f;
		float threshold = 300.0f;
		if (distance > threshold) {
			float k1 = 0.001f;
			float k2 = 0.0002f;
			attenuation = 1.0f / (1.0f + k1 * (distance - threshold) + k2 * pow(distance - threshold, 2));
		}

		// Diffuse lighting
		float diff = max(dot(normal, lightDirection), 0.0);
		vec3 diffuse = diff * lightIntensities[i] * attenuation;

		// Ambient lighting
		vec3 ambient = vec3(0.01);	// Ambient component

		// Combine diffuse and ambient
		vec3 lighting = diffuse + ambient;

		// Shadow calculations
		vec4 fragPosLightSpace = lightSpaceMatrices[i] * model * vec4(worldPosition, 1.0);
		vec3 lightCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
		lightCoords = lightCoords * 0.5 + 0.5;

		float shadow;
		if (lightCoords.x < 0.0 || lightCoords.x > 1.0 ||
		lightCoords.y < 0.0 || lightCoords.y > 1.0 ||
		lightCoords.z > 1.0) {
			shadow = 1.0; // Not in shadow
		} else {
			float closestDepth = texture(shadowMapArray, lightCoords.xy).r;
			float bias = 0; //max(0.05 * (1.0 - dot(normal, lightDirection)), 0.005);
			vec2 texelSize = 1.0 / textureSize(shadowMapArray, 0).xz;
			for(int x = -1; x <= 1; ++x) {
				for(int y = -1; y <= 1; ++y) {
					float pcfDepth = texture(shadowMapArray, lightCoords.xy + vec2(x, y) * texelSize).r;
					shadow += lightCoords.z - bias >= pcfDepth ? 0.2 : 1.0;
				}
			}
			shadow /= 9.0;
		}
		res += lighting * shadow;

	}
	// Tone mapping
	vec3 exposedColor = res * exposure;
	vec3 toneMappedColor = exposedColor / (exposedColor + vec3(1.0));
	finalColor = (texture(textureSampler, uv).rgba * baseColorFactor * vec4(pow(toneMappedColor, vec3(1.0 / 2.2)), 1.0));
}
