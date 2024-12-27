#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <random>
#include <render/shader.h>

#include <vector>
#include <iostream>
#define _USE_MATH_DEFINES
#include <cube.h>

#include <math.h>
#include <skybox.h>
#include <glm/gtc/type_ptr.inl>

#include <texture.h>

#include "asset.h"

Cube::Cube(GLuint programID, const std::vector<glm::mat4>& instanceTransforms, const char *texture_file_path) {
    this->instanceTransforms = instanceTransforms;
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

    for (int i = 0; i < 24; i++) {
        uv_buffer_data[2*i+1] *= 5;
    }

    // Create a vertex buffer object to store the UV data
    glGenBuffers(1, &uvBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uv_buffer_data), uv_buffer_data, GL_STATIC_DRAW);

    // Create an index buffer object to store the index data that defines triangle faces
    glGenBuffers(1, &indexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data), index_buffer_data, GL_STATIC_DRAW);
    textureID = LoadTextureTileBox(texture_file_path);
    shininessID = glGetUniformLocation(programID, "shininess");
    this->programID = programID;

    cameraMatrixID = glGetUniformLocation(programID, "camera");

    textureID = LoadTextureTileBox(texture_file_path);

    textureSamplerID = glGetUniformLocation(programID, "textureSampler");
    baseColorFactorID = glGetUniformLocation(programID, "baseColorFactor");
    isLightID = glGetUniformLocation(programID, "isLight");

    glGenBuffers(1, &instanceBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, instanceBufferID);
    glBufferData(GL_ARRAY_BUFFER, instanceTransforms.size() * sizeof(glm::mat4), instanceTransforms.data(), GL_STATIC_DRAW);

    for (int i = 0; i < 4; i++) { // mat4 occupies 4 vec4s
        glEnableVertexAttribArray(3 + i);
        glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(i * sizeof(glm::vec4)));
        glVertexAttribDivisor(3 + i, 1); // One per instance
    }
}

void Cube::updateInstances(const std::vector<glm::mat4>& instanceTransforms) {
    glBindBuffer(GL_ARRAY_BUFFER, instanceBufferID);

    static int currentBufferSize = 0;
    int newSize = instanceTransforms.size() * sizeof(glm::mat4);
    if (newSize > currentBufferSize) {
        glBufferData(GL_ARRAY_BUFFER, newSize, nullptr, GL_DYNAMIC_DRAW);
        currentBufferSize = newSize;
    }
    glBufferSubData(GL_ARRAY_BUFFER, 0, newSize, instanceTransforms.data());
}

void Cube::render(glm::mat4 cameraMatrix) {
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

    // Enable UV buffer and texture sampler
    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
    glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, 0, 0);

    // Set textureSampler to use texture unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glUniform1i(textureSamplerID, 0);

    // Bind the instance buffer
    glBindBuffer(GL_ARRAY_BUFFER, instanceBufferID);
    for (int i = 0; i < 4; ++i) {
        glEnableVertexAttribArray(3 + i);
        glVertexAttribPointer(3 + i, 4, GL_FLOAT, GL_FALSE, sizeof(glm::mat4), (void*)(i * sizeof(glm::vec4)));
        glVertexAttribDivisor(3 + i, 1);
    }

    // Set base colour factor to opaque
    glm::vec4 baseColorFactor = glm::vec4(1.0);
    glUniform4fv(baseColorFactorID, 1, &baseColorFactor[0]);
    glUniform1i(isLightID, 0);
    glUniform1f(shininessID, 4.0f);

    glDrawElementsInstanced(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void*)0,
        instanceTransforms.size());

    for (int i = 0; i < 4; ++i) {
        glDisableVertexAttribArray(3 + i);
    }
    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    glDisableVertexAttribArray(2);
}

void Cube::renderDepth(GLuint programID, GLuint lightMatID, const glm::mat4& lightSpaceMatrix){
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