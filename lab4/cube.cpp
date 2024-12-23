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
#include <texture.h>

#include <asset.h>
#include <stb_image_write.h>

std::vector<Cube*> cubes;
glm::mat4 calculateLightSpaceMatrix(glm::vec3 lightPosition, glm::vec3 lightTarget);

Cube::Cube(glm::vec3 position, glm::vec3 scale, const char *texture_file_path) {
    this->position = position;
    this->scale = scale;

    shadowFBOs.resize(lights.size());
    shadowMaps.resize(lights.size());
    for (int i = 0; i < lights.size(); ++i) {
        // Create framebuffer for shadow map
        glGenFramebuffers(1, &shadowFBOs[i]);

        // Create texture for shadow map
        glGenTextures(1, &shadowMaps[i]);
        glBindTexture(GL_TEXTURE_2D, shadowMaps[i]);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, 1024, 1024, 0, GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

        // Attach shadow map to the framebuffer
        glBindFramebuffer(GL_FRAMEBUFFER, shadowFBOs[i]);
        glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowMaps[i], 0);
        glDrawBuffer(GL_NONE);
        glReadBuffer(GL_NONE);
        if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
            std::cerr << "Error creating shadow framebuffer for light " << i << std::endl;
        }
    }
    shadowShaderID = LoadShadersFromFile("../lab4/shader/depth.vert", "../lab4/shader/depth.frag");
    if (shadowShaderID == 0) {
        std::cerr << "Failed to load shadow shaders." << std::endl;
    }
    shadowModelMatrixID = glGetUniformLocation(shadowShaderID, "model");
    shadowLightSpaceMatrixID = glGetUniformLocation(shadowShaderID, "lightSpaceMatrix");

    for (int i = 0; i < lights.size(); ++i) {
        glm::vec3 lightTarget = glm::vec3(0.0f); // Assuming the light is looking at the origin
        lightSpaceMatrices.push_back(calculateLightSpaceMatrix(lights[i].lightPosition, lightTarget));
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);


    // Create a vertex array object
    glGenVertexArrays(1, &vertexArrayID);
    glBindVertexArray(vertexArrayID);

    // Create a vertex buffer object to store the vertex data
    glGenBuffers(1, &vertexBufferID);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertex_buffer_data), vertex_buffer_data, GL_STATIC_DRAW);

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
    //mvpMatrixID = glGetUniformLocation(programID, "MVP");
    modelMatrixID = glGetUniformLocation(programID, "model");
    viewMatrixID = glGetUniformLocation(programID, "view");
    projectionMatrixID = glGetUniformLocation(programID, "projection");
    lightPositionID = glGetUniformLocation(programID, "lightPosition");
    lightIntensityID = glGetUniformLocation(programID, "lightIntensity");
    textureID = LoadTextureTileBox(texture_file_path);
    textureSamplerID = glGetUniformLocation(programID,"textureSampler");
    numLights = glGetUniformLocation(programID, "numLights");
    cubes.push_back(this); //currently unused
}

void Cube::render(glm::mat4 cameraMatrix) {
    renderShadows(lightSpaceMatrices);
    glUseProgram(programID);
    glBindVertexArray(vertexArrayID);

    // Position
    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // Normal
    glEnableVertexAttribArray(1);
    glBindBuffer(GL_ARRAY_BUFFER, normalBufferID);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 0, 0);

    // UV
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

    for (int i = 0; i < lightSpaceMatrices.size(); ++i) {
        glUniformMatrix4fv(glGetUniformLocation(programID, ("lightSpaceMatrices[" + std::to_string(i) + "]").c_str()),
                           1, GL_FALSE, glm::value_ptr(lightSpaceMatrices[i]));

        glActiveTexture(GL_TEXTURE0 + i);
        glBindTexture(GL_TEXTURE_2D, shadowMaps[i]);
        glUniform1i(glGetUniformLocation(programID, ("shadowMaps[" + std::to_string(i) + "]").c_str()), i + 1);

    }

    int numLights = lights.size();

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
    glUniform3fv(glGetUniformLocation(programID, "viewPos"), 1, glm::value_ptr(cameraPos));

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

}

void Cube::renderShadows(const std::vector<glm::mat4>& lightSpaceMatrices) {
    glEnable(GL_CULL_FACE); // Enable face culling
    glCullFace(GL_BACK);   // Cull back faces
    for (int i = 0; i < lightSpaceMatrices.size(); ++i) {
        glBindFramebuffer(GL_FRAMEBUFFER, shadowFBOs[i]);
        glClear(GL_DEPTH_BUFFER_BIT);

        glEnable(GL_DEPTH_TEST);
        glDepthFunc(GL_LESS);

        // Use shadow shader
        glUseProgram(shadowShaderID);

        glActiveTexture(GL_TEXTURE0 + i); // Activate texture unit for the shadow map
        glBindTexture(GL_TEXTURE_2D, shadowMaps[i]); // Bind the shadow map texture

        // Pass the light-space matrix and model matrix
        glUniformMatrix4fv(shadowLightSpaceMatrixID, 1, GL_FALSE, glm::value_ptr(lightSpaceMatrices[i]));

        for (Cube* cube : cubes) { // Render all cubes into the shadow map
            glm::mat4 modelMatrix = glm::mat4(1.0f);
            modelMatrix = glm::translate(modelMatrix, cube->position);
            modelMatrix = glm::scale(modelMatrix, cube->scale);
            glUniformMatrix4fv(shadowModelMatrixID, 1, GL_FALSE, glm::value_ptr(modelMatrix));
            glBindVertexArray(cube->vertexArrayID);
            glDrawElements(GL_TRIANGLES, 36, GL_UNSIGNED_INT, (void*)0);
        }
    }
    glDisable(GL_CULL_FACE); // Disable face culling after shadow pass
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

glm::mat4 calculateLightSpaceMatrix(glm::vec3 lightPosition, glm::vec3 lightTarget) {
    float farPlane = 2000.0f;
    float nearPlane = 10.0f;
    glm::mat4 lightViewMatrix = glm::lookAt(lightPosition, lightTarget, glm::vec3(0.0f, 1.0f, 0.0f));
    glm::mat4 lightProjectionMatrix = glm::ortho(-10.0f, 10.0f, -10.0f, 10.0f, nearPlane, farPlane);

    return lightProjectionMatrix * lightViewMatrix;
}

// This function retrieves and stores the depth map of the default frame buffer
// or a particular frame buffer (indicated by FBO ID) to a PNG image.
void saveDepthTexture(GLuint fbo, std::string filename) {
    int width = 1024;
    int height = windowHeight;

    int channels = 3;

    std::vector<float> depth(width * height);
    glBindFramebuffer(GL_FRAMEBUFFER, fbo);
    glReadBuffer(GL_DEPTH_COMPONENT);
    glReadPixels(0, 0, width, height, GL_DEPTH_COMPONENT, GL_FLOAT, depth.data());
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    std::vector<unsigned char> img(width * height * 3);
    for (int i = 0; i < width * height; ++i) img[3*i] = img[3*i+1] = img[3*i+2] = depth[i] * 255;

    stbi_write_png(filename.c_str(), width, height, channels, img.data(), width * channels);
}


