#include "lighting.h"

#include <iostream>
#include <render/shader.h>

#include "asset.h"

void lighting::initialize(GLuint programID, int shadowMapWidth, int shadowMapHeight) {
    this->programID = programID;
    depthProgramID = LoadShadersFromFile("../lab4/shader/depth.vert", "../lab4/shader/depth.frag");
    if (depthProgramID == 0) {
        std::cerr << "Failed to load depth shaders." << std::endl;
    }

    // Initialize light and shadow uniform locations
    lightPositionID = glGetUniformLocation(programID, "lightPosition");
    lightIntensityID = glGetUniformLocation(programID, "lightIntensity");
    lightExposureID = glGetUniformLocation(programID, "exposure");
    lightSpaceMatrixID = glGetUniformLocation(programID, "lightSpaceMatrix");
    depthMatrixID = glGetUniformLocation(depthProgramID, "MVP");

    // Record shadow map size
    this->shadowMapWidth = shadowMapWidth;
    this->shadowMapHeight = shadowMapHeight;

    // Create shadow texture
    glGenTextures(1, &shadowTexture);
    glBindTexture(GL_TEXTURE_2D, shadowTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, shadowMapWidth, shadowMapHeight, 0, GL_DEPTH_COMPONENT, GL_FLOAT, nullptr);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);

    // Create shadow framebuffer
    glGenFramebuffers(1, &shadowFBO);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadowTexture, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);

    // Check framebuffer completeness
    if (glCheckFramebufferStatus(GL_FRAMEBUFFER) != GL_FRAMEBUFFER_COMPLETE) {
        std::cerr << "Error: Shadow framebuffer is not complete!" << std::endl;
    }
    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void lighting::setLightPosition(glm::vec3 lightPosition, glm::vec3 lightIntensity, float exposure) {
    this->lightPosition = lightPosition;
    this->lightIntensity = lightIntensity;
    this->lightExposure = exposure;
    for (int i = 0; i < 125; ++i) {
        this->lightIntensity[i] /= 1.1f;
    }
}

void lighting::shadowPass(glm::mat4 lightSpaceMatrix, std::vector<Asset> assets, std::vector<Plane> planes) {
    // Record Light Space Matrix
    this->lightSpaceMatrix = lightSpaceMatrix;

    // Perform Shadow pass
    glUseProgram(depthProgramID);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    for (auto& asset : assets) asset.renderDepth(depthProgramID, depthMatrixID, lightSpaceMatrix);
    //for (auto& plane : planes) plane.renderDepth(depthProgramID, depthMatrixID, lightSpaceMatrix);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
    glUseProgram(0);

    if (saveDepth) saveDepthTexture(shadowFBO, "depth.png");
    saveDepth = false;
}

void lighting::shadowPass(glm::mat4 lightSpaceMatrix, GLuint vertexBufferID, GLuint indexBufferID, int indexCount) {
    // Record Light Space Matrix
    this->lightSpaceMatrix = lightSpaceMatrix;

    // Perform Shadow pass
    glUseProgram(depthProgramID);
    glBindFramebuffer(GL_FRAMEBUFFER, shadowFBO);
    glClear(GL_DEPTH_BUFFER_BIT);

    glm::mat4 lightMVP = lightSpaceMatrix;
    glUniformMatrix4fv(depthMatrixID, 1, GL_FALSE, &lightMVP[0][0]);

    glEnableVertexAttribArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, vertexBufferID);
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 0, 0);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, indexBufferID);
    glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, (void*)0);
    glDisableVertexAttribArray(0);

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}

void lighting::lightSetup() {
    glUseProgram(programID);

    GLuint shadowMapID = glGetUniformLocation(programID, "shadowMap");
    glActiveTexture(GL_TEXTURE1);
    glBindTexture(GL_TEXTURE_2D, shadowTexture);
    glUniform1i(shadowMapID, 1);

    glUniform3fv(lightPositionID, 1, &lightPosition[0]);
    glUniform3fv(lightIntensityID, 1, &lightIntensity[0]);
    glUniform1f(lightExposureID, lightExposure);
    glUniformMatrix4fv(lightSpaceMatrixID, 1, GL_FALSE, &lightSpaceMatrix[0][0]);
}



