layout(location = 0) in vec3 vertexPosition;
layout(location = 1) in vec3 vertexNormal;
layout(location = 2) in vec2 vertexUV;
// Add joint indices and weights for linear blend skinning
layout(location = 3) in vec4 jointIndices; // Indices of up to 4 joints
layout(location = 4) in vec4 jointWeights; // Corresponding weights of each joint

// Output data, to be interpolated for each fragment
out vec3 worldPosition;
out vec3 worldNormal;

uniform mat4 MVP;
uniform mat4 jointMatrices[100]; // Adjust size to fit the number of joints in your skin

void main() {
    // Compute skinning matrix
    vec4 normalizedWeights = jointWeights / dot(jointWeights, vec4(1.0));
    mat4 skinMatrix =
    normalizedWeights.x * jointMatrices[int(jointIndices.x)] +
    normalizedWeights.y * jointMatrices[int(jointIndices.y)] +
    normalizedWeights.z * jointMatrices[int(jointIndices.z)] +
    normalizedWeights.w * jointMatrices[int(jointIndices.w)];

    // Transform vertex using skinning matrix
    gl_Position =  MVP * skinMatrix * vec4(vertexPosition, 1.0);

    // World-space geometry

    worldPosition = (skinMatrix * vec4(vertexPosition, 1.0)).xyz;
    worldNormal = normalize(mat3(transpose(inverse(skinMatrix))) * vertexNormal);
}