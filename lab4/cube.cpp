#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <random>

#include <render/shader.h>

//#define STB_IMAGE_IMPLEMENTATION
#include <stb/stb_image.h>

#include <vector>
#include <iostream>
#define _USE_MATH_DEFINES
#include <cube.h>


#include <math.h>
#include <skybox.h>
#include <glm/gtc/type_ptr.inl>

#include <light.h>

Cube::Cube(glm::vec3 position, glm::vec3 scale, const char *texture_file_path) {
    this->position = position;
    this->scale = scale;
    // Create a vertex array object
    glGenVertexArrays(1, &vertexArrayID);
    glBindVertexArray(vertexArrayID);

    // Create a vertex buffer object to store the vertex data
    glGenBuffers(1, &vertexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);

    // Create a vertex buffer object to store the color data
    glGenBuffers(1, &colorBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(color_buffer_data), color_buffer_data, GL_STATIC_DRAW);

    // Create a vertex buffer object to store the vertex normals
    glGenBuffers(1, &normalBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(normal_buffer_data), normal_buffer_data, GL_STATIC_DRAW);

    glGenBuffers(1, &uvBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(uv_buffer_data), uv_buffer_data,
GL_STATIC_DRAW);

    // Create an index buffer object to store the index data that defines triangle faces
    glGenBuffers(1, &indexBufferID);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(index_buffer_data), index_buffer_data, GL_STATIC_DRAW);

    programID = LoadShadersFromFile("../lab4/shader/cube.vert", "../lab4/shader/cube.frag");
    if (programID == 0)
    {
        std::cerr << "Failed to load shaders." << std::endl;
    }
    mvpMatrixID = glGetUniformLocation(programID, "MVP");
    lightPositionID = glGetUniformLocation(programID, "lightPosition");
    lightIntensityID = glGetUniformLocation(programID, "lightIntensity");
    textureID = LoadTextureTileBox(texture_file_path);
    textureSamplerID = glGetUniformLocation(programID,"textureSampler");
    numLights = glGetUniformLocation(programID, "numLights");
}

void Cube::render(glm::mat4 cameraMatrix) {
    glUseProgram(programID);
    glBindVertexArray(vertexArrayID);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, colorBufferID);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glEnableVertexAttribArray(2);
    glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 0, 0);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    glm::mat4 modelMatrix = glm::mat4();
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::scale(modelMatrix, scale);
    glm::mat4 mvp = cameraMatrix * modelMatrix;
    glUniformMatrix4fv(mvpMatrixID, 1, GL_FALSE, &mvp[0][0]);


    int numLights = lights.size();

    if (numLights == 0) {
        glm::vec3 ambientColor = glm::vec3(0.2f, 0.2f, 0.2f); // Dim gray light
        //glUniform3fv(glGetUniformLocation(programID, "ambientColor"), 1, &ambientColor[0]);
    }

    glUniform1i(glGetUniformLocation(programID, "numLights"), numLights);

    // Upload light positions and intensities only if numLights > 0
    if (numLights > 0) {
        std::vector<glm::vec3> lightPositions, lightIntensities;
        for (const auto& light : lights) {
            lightPositions.push_back(light.lightPosition);
            lightIntensities.push_back(light.lightIntensity);
        }
        glUniform3fv(glGetUniformLocation(programID, "lightPositions"), numLights, &lightPositions[0][0]);
        glUniform3fv(glGetUniformLocation(programID, "lightIntensities"), numLights, &lightIntensities[0][0]);
    }

    // TODO: Enable UV buffer and texture sampler
    // ------------------------------------------
    // ------------------------------------------
    glEnableVertexAttribArray(3);
    glBindBuffer(GL_ARRAY_BUFFER, uvBufferID);
    glVertexAttribPointer(3, 2, GL_FLOAT, GL_FALSE, 0, 0);

    // Set textureSampler to use texture unit 0
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, textureID);
    glUniform1i(textureSamplerID, 0);


    // Draw the box
    glDrawElements(
        GL_TRIANGLES,      // mode
        36,    			   // number of indices
        GL_UNSIGNED_INT,   // type
        (void*)0           // element array buffer offset
    );

    glDisableVertexAttribArray(0);
    glDisableVertexAttribArray(1);
    //glDisableVertexAttribArray(2);
    //glDepthMask(GL_FALSE);
    //glUseProgram(0);
    //glBindVertexArray(0);

}
