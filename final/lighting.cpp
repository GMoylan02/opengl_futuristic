//
// Created by eyeba on 23/12/2024.
//

#include "lighting.h"

#include <iostream>
#include <glm/detail/func_geometric.inl>
#include <glm/gtc/matrix_transform.hpp>
#include <render/shader.h>



lighting::lighting(GLuint programID, int shadowMapWidth, int shadowMapHeight) {
    // Set program IDs
    programID = LoadShadersFromFile("../final/shader/model.vert", "../final/shader/model.frag");
    if (programID == 0)
    {
        std::cerr << "Failed to load main shaders." << std::endl;
    }

    depthProgramID = LoadShadersFromFile("../final/shader/depth.vert", "../final/shader/depth.frag");
    if (depthProgramID == 0) {
        std::cerr << "Failed to load depth shaders." << std::endl;
    }

    // Record shadow map size
    this->shadowMapWidth = shadowMapWidth;
    this->shadowMapHeight = shadowMapHeight;

    // Create shadow texture
    glGenTextures(1, &shadowMapArray);
    glBindTexture(GL_TEXTURE_2D, shadowMapArray);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Check framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: Shadow framebuffer is not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void lighting::add(glm::vec3 position, glm::vec3 intensity, float exposure) {
    for (const auto& light : lightSources) {
        if (glm::distance(light.position, position) < 0.1f) {
            return;
        }
    }

    LightSource light;
    light.position = position;
    light.intensity = intensity;
    light.exposure = exposure;

    // Initialize shadow framebuffer (we're using the shared shadow map array)
    glGenFramebuffers(1, &light.shadowFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, light.shadowFBO);

    glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowMapArray, 0, lightSources.size());
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    lightSources.push_back(light);
}
/*
void lighting::setLightPosition(glm::vec3 lightPosition, glm::vec3 lightIntensity, float exposure) {
    this->lightPosition = lightPosition;
    this->lightIntensity = lightIntensity;
    lightExposure = exposure;
}
*/

void lighting::pruneLights(int chunkX, int chunkZ) {
    std::vector<LightSource> newLights;
    //tune
    for (const auto& light : lightSources) {
        int lightChunkX = light.position.x / CHUNK_SIZE;
        int lightChunkZ = light.position.z / CHUNK_SIZE;
        if (abs(chunkX-lightChunkX) <= 1 && abs(chunkZ-lightChunkZ) <= 1) {
            newLights.push_back(light);
        }
    }
    lightSources = newLights;
}

void lighting::shadowPass(glm::mat4 lightProjection, std::vector<Asset> assets,  std::vector<Cube> cubes, std::vector<Plane> planes) {
    glUseProgram(depthProgramID);
    for (int i = 0; i < lightSources.size(); i++) {
        LightSource l = lightSources[i];
        glm::vec3 lookAt = glm::vec3(l.position.x, l.position.y - 1, l.position.z);
        glm::mat4 lightView = glm::lookAt(l.position, lookAt, glm::vec3(0, 0, 1));
        l.lightSpaceMatrix = lightProjection * lightView;
        glBindFramebuffer(GL_FRAMEBUFFER, l.shadowFBO);
        glFramebufferTextureLayer(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, shadowMapArray, 0, i);
        glClear(GL_DEPTH_BUFFER_BIT);
        for (auto& asset : assets) asset.renderDepth(depthProgramID, glGetUniformLocation(depthProgramID, "lightSpace"), l.lightSpaceMatrix);
        for (auto& cube : cubes) cube.renderDepth(depthProgramID, glGetUniformLocation(depthProgramID, "lightSpace"), l.lightSpaceMatrix);
        for (auto& plane : planes) plane.renderDepth(depthProgramID, glGetUniformLocation(depthProgramID, "lightSpace"), l.lightSpaceMatrix);
        glBindFramebuffer(GL_FRAMEBUFFER, 0);
    }

    glUseProgram(0);
}

void lighting::prepareLighting() {
    glUseProgram(programID);
    for (int i = 0; i < lightSources.size(); i++) {
        std::string id = std::to_string(i);
        glUniform3fv(glGetUniformLocation(programID, ("lightPositions[" + id + "]").c_str()),
            1, &lightSources[i].position[0]);
        glUniform3fv(glGetUniformLocation(programID, ("lightIntensities[" + id + "]").c_str()),
            1, &lightSources[i].intensity[0]);
        glUniform1f(glGetUniformLocation(programID, ("lightExposures[" + id + "]").c_str()),
            lightSources[i].exposure);
        glUniformMatrix4fv(glGetUniformLocation(programID, ("lightSpaceMatrices[" + id + "]").c_str()),
            1, GL_FALSE, &lightSources[i].lightSpaceMatrix[0][0]);
    }
    GLuint shadowMapArrayID = glGetUniformLocation(programID, "shadowMapArray");
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D_ARRAY, shadowMapArray);
    glUniform1i(shadowMapArrayID, 1);
    glUniform3fv(glGetUniformLocation(programID, "cameraPosition"), 1, &cameraPos[0]);
    glUniform1i(glGetUniformLocation(programID, "lightCount"), lightSources.size());
}


