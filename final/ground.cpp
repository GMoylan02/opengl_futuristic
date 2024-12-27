
#include "ground.h"

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.inl>
#include <render/shader.h>

#include "asset.h"
#include "texture.h"

Plane::Plane(glm::vec3 position, glm::vec3 scale, const char *texture_file_path) {
	this->position = position;
	this->scale = scale;

	// Model transform
	modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, position);
	modelMatrix = glm::scale(modelMatrix, scale);

	// Create a vertex array object
	glGenVertexArrays(1, &vertexArrayID);
	glBindVertexArray(vertexArrayID);

	// Create a vertex buffer object to store the vertex data
	glGenBuffers(1, &vertexBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);

	// Create a vertex buffer object to store the normal data
	glGenBuffers(1, &normalBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(normal_buffer_data), normal_buffer_data, GL_STATIC_DRAW);

	// Create a vertex buffer object to store the UV data
	glGenBuffers(1, &uvBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
	glBufferData(GL_ARRAY_BUFFER, sizeof(uv_buffer_data), uv_buffer_data, GL_STATIC_DRAW);

	// Create an index buffer object to store the index data that defines triangle faces
	glGenBuffers(1, &indexBufferID);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data), index_buffer_data, GL_STATIC_DRAW);

	// Create and compile our GLSL program from the shaders
	programID = LoadShadersFromFile("../final/shader/ground.vert", "../final/shader/ground.frag");
	if (programID == 0)
	{
		std::cerr << "Failed to load shaders." << std::endl;
	}

	// Get a handle for our "MVP" uniforms
	cameraMatrixID = glGetUniformLocation(programID, "camera");
	transformMatrixID = glGetUniformLocation(programID, "transform");

	// Get a handle for our Model Matrix uniform
	modelMatrixID = glGetUniformLocation(programID, "modelMatrix");

	textureID = LoadTextureTileBox(texture_file_path);

	// Get a handle for our "textureSampler" uniform
	textureSamplerID = glGetUniformLocation(programID, "textureSampler");
	baseColorFactorID = glGetUniformLocation(programID, "baseColorFactor");
	isLightID = glGetUniformLocation(programID, "isLight");
	shininessID = glGetUniformLocation(programID, "shininess");
	cameraPosID = glGetUniformLocation(programID, "cameraPos");

	glGenBuffers(1, &instanceBufferID);
	glBindBuffer(GL_ARRAY_BUFFER, instanceBufferID);
	glBufferData(GL_ARRAY_BUFFER, instanceTransforms.size() * sizeof(glm::mat4),
		instanceTransforms.data(), GL_STATIC_DRAW);

	for (int i = 0; i < 4; i++) {
		glEnableVertexAttribArray(3 + i);
		glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4),
			(void*)(i * sizeof(glm::vec4)));
		glVertexAttribDivisor(3 + i, 1); // One per instance
	}
}

void Plane::updateInstances(const std::vector<glm::mat4>& instanceTransforms) {
	glBindBuffer(GL_ARRAY_BUFFER, instanceBufferID);
	static int currentBufferSize = 0;
	int size = instanceTransforms.size() * sizeof(glm::mat4);
	if (size > currentBufferSize) {
		glBufferData(GL_ARRAY_BUFFER, size, nullptr, GL_DYNAMIC_DRAW);
		currentBufferSize = size;
	}
	glBufferSubData(GL_ARRAY_BUFFER, 0, size, instanceTransforms.data());
}

void Plane::render(glm::mat4 cameraMatrix) {
	glUseProgram(programID);

	glBindVertexArray(vertexArrayID);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glEnableVertexAttribArray(1);
	glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

	// Pass in model-view-projection matrix
	glUniformMatrix4fv(cameraMatrixID, 1, GL_FALSE, &cameraMatrix[0][0]);
	glUniformMatrix4fv(transformMatrixID, 1, GL_FALSE, &modelMatrix[0][0]);
	glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMatrix[0][0]);

	// Enable UV buffer and texture sampler
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

	// Set textureSampler to use texture unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glUniform1i(textureSamplerID, 0);

	glBindBuffer(GL_ARRAY_BUFFER, instanceBufferID);
	for (int i = 0; i < 4; i++) {
		glEnableVertexAttribArray(3 + i);
		glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4),
			(void*)(i * sizeof(glm::vec4)));
		glVertexAttribDivisor(3 + i, 1);
	}

	// Set base colour factor to opaque
	glm::vec4 baseColorFactor = glm::vec4(1.0);
	glUniform4fv(baseColorFactorID, 1, &baseColorFactor[0]);
	glUniform1i(isLightID, 0);
	glUniform1f(shininessID, 4.0f);
	glUniform3fv(cameraPosID, 1, &cameraPos[0]);

	glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void*)0,
		instanceTransforms.size());

	for (int i = 0; i < 4; ++i) {
		glDisableVertexAttribArray(3 + i);
	}
	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
}


void Plane::renderDepth(GLuint programID, GLuint lightMatID, const glm::mat4& lightSpaceMatrix) {
	glUseProgram(programID);

	// Pass the MVP matrix to the shader
	glUniformMatrix4fv(lightMatID, 1, GL_FALSE, &lightSpaceMatrix[0][0]);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

	glBindBuffer(GL_ARRAY_BUFFER, instanceBufferID);
	for (int i = 0; i < 4; i++) {
		glEnableVertexAttribArray(3 + i);
		glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(i * sizeof(glm::vec4)));
		glVertexAttribDivisor(3 + i, 1);
	}

	glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void*)0, instanceTransforms.size());
	glDisableVertexAttribArray(0);

	for (int i = 0; i < 4; i++) {
		glDisableVertexAttribArray(3 + i);
	}
	glBindVertexArray(0);
	glUseProgram(0);
}

void Plane::cleanup() {
	glDeleteBuffers(1, &vertexBufferID);
	glDeleteBuffers(1, &normalBufferID);
	glDeleteBuffers(1, &indexBufferID);
	glDeleteVertexArrays(1, &vertexArrayID);
	glDeleteBuffers(1, &uvBufferID);
	glDeleteTextures(1, &textureID);
	glDeleteProgram(programID);
}


