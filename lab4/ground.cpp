
#include "ground.h"

#include <iostream>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.inl>
#include <render/shader.h>

#include "asset.h"
#include "light.h"
#include "texture.h"

//extern std::vector<Plane*> planes;

Plane::Plane(glm::vec3 position, glm::vec3 scale) {
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
	programID = LoadShadersFromFile("../lab4/shader/asset.vert", "../lab4/shader/asset.frag");
	if (programID == 0)
	{
		std::cerr << "Failed to load shaders." << std::endl;
	}

	// Load a random texture into the GPU memory
	textureID = LoadTextureTileBox("../lab4/assets/ground.jpg");
	modelMatrixID = glGetUniformLocation(programID, "model");
	viewMatrixID = glGetUniformLocation(programID, "view");
	projectionMatrixID = glGetUniformLocation(programID, "projection");
	lightPositionID = glGetUniformLocation(programID, "lightPosition");
	lightIntensityID = glGetUniformLocation(programID, "lightIntensity");
	textureSamplerID = glGetUniformLocation(programID,"textureSampler");
	numLights = glGetUniformLocation(programID, "numLights");
//	planes.push_back(this);
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

	// Enable UV buffer and texture sampler
	glEnableVertexAttribArray(2);
	glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);

	glm::mat4 modelMatrix = glm::mat4(1.0f);
	modelMatrix = glm::translate(modelMatrix, position);
	modelMatrix = glm::scale(modelMatrix, scale);

	glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, glm::value_ptr(modelMatrix));
	glUniformMatrix4fv(viewMatrixID, 1, GL_FALSE, glm::value_ptr(cameraMatrix)); // Assume cameraMatrix is view
	glUniformMatrix4fv(projectionMatrixID, 1, GL_FALSE, glm::value_ptr(projectionMatrix)); // Pass the projection matrix

	passLightsToShader(programID); //todo this might be buggy
	glUniform3fv(glGetUniformLocation(programID, "viewPos"), 1, glm::value_ptr(cameraPos));

	// Set textureSampler to use texture unit 0
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, textureID);
	glUniform1i(textureSamplerID, 0);

	/*
	// Set base colour factor to opaque
	glm::vec4 baseColorFactor = glm::vec4(1.0);
	glUniform4fv(baseColorFactorID, 1, &baseColorFactor[0]);
	glUniform1i(isLightID, 0);
	*/

	// Draw the box
	glDrawElements(
		GL_TRIANGLES,      // mode
		36,    			   // number of indices
		GL_UNSIGNED_INT,   // type
		(void*)0           // element array buffer offset
	);

	glDisableVertexAttribArray(0);
	glDisableVertexAttribArray(1);
	glDisableVertexAttribArray(2);
}

void Plane::renderDepth(GLuint programID, GLuint mvpMatrixID, const glm::mat4& lightSpaceMatrix) {
	glUseProgram(programID);

	// Combine transformations with the camera matrix
	glm::mat4 mvpMatrix = lightSpaceMatrix * modelMatrix;

	// Pass the updated MVP matrix to the shader
	glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvpMatrix[0][0]);

	glEnableVertexAttribArray(0);
	glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
	glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void*)0);
	glDisableVertexAttribArray(0);

	// Reset state
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