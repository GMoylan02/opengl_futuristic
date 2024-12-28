#version 330 core

in vec3 worldPosition;
in vec3 worldNormal;
in vec2 uv;

uniform sampler2D textureSampler;
uniform vec4 baseColorFactor;

out vec4 finalColor;

uniform vec3 lightPosition;
uniform vec3 lightIntensity;
uniform float exposure;
uniform sampler2D shadowMap;
uniform mat4 lightSpaceMatrix;
uniform mat4 modelMatrix;

vec3 calculateLighting(vec3 normal, vec3 fragPosition, vec3 lightDirection, float distance);
float calculateShadow(vec3 lightCoords, vec3 normal, vec3 lightDirection);
vec3 applyToneMapping(vec3 color);

void main() {
	vec3 normal = normalize(worldNormal);
	vec3 fragPosition = vec3(modelMatrix * vec4(worldPosition, 1.0));
	vec3 lightDirection = normalize(lightPosition - fragPosition);
	float distance = length(lightPosition - fragPosition);

	vec3 lighting = calculateLighting(normal, fragPosition, lightDirection, distance);

	vec4 fragPosLightSpace = lightSpaceMatrix * modelMatrix * vec4(worldPosition, 1.0);
	vec3 lightCoords = fragPosLightSpace.xyz / fragPosLightSpace.w;
	lightCoords = lightCoords * 0.5 + 0.5;

	float shadow = calculateShadow(lightCoords, normal, lightDirection);
	lighting *= shadow;

	vec3 exposedColor = lighting * exposure;
	vec3 toneMappedColor = applyToneMapping(exposedColor);

	finalColor = texture(textureSampler, uv) * baseColorFactor * vec4(pow(toneMappedColor, vec3(1.0 / 2.2)), 1.0);
}

vec3 calculateLighting(vec3 normal, vec3 fragPosition, vec3 lightDirection, float distance) {
	float attenuation = 1.0f;
	float threshold = 300.0f;
	if (distance > threshold) {
		float k1 = 0.001f;
		float k2 = 0.0002f;
		attenuation = 1.0f / (1.0f + k1 * (distance - threshold) + k2 * pow(distance - threshold, 2));
	}

	float diff = max(dot(normal, lightDirection), 0.0);
	vec3 diffuse = diff * lightIntensity * attenuation;

	vec3 ambient = vec3(0.01);

	return diffuse + ambient;
}

float calculateShadow(vec3 lightCoords, vec3 normal, vec3 lightDirection) {
	if (lightCoords.x < 0.0 || lightCoords.x > 1.0 ||
	lightCoords.y < 0.0 || lightCoords.y > 1.0 ||
	lightCoords.z > 1.0) {
		return 1.0;
	}

	float shadow = 0.0;
	float closestDepth = texture(shadowMap, lightCoords.xy).r;
	float bias = 0;
	vec2 texelSize = 1.0 / textureSize(shadowMap, 0);

	for (int x = -1; x <= 1; ++x) {
		for (int y = -1; y <= 1; ++y) {
			float pcfDepth = texture(shadowMap, lightCoords.xy + vec2(x, y) * texelSize).r;
			shadow += lightCoords.z - bias >= pcfDepth ? 0.2 : 1.0;
		}
	}
	return shadow / 9.0;
}

vec3 applyToneMapping(vec3 color) {
	return color / (color + vec3(1.0));
}
