//
// Created by eyeba on 23/12/2024.
//

#ifndef LIGHTING_H
#define LIGHTING_H
#include <vector>
#include <glm/detail/type_mat.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include <glm/detail/type_vec.hpp>
#include <glm/detail/type_vec3.hpp>

#include "asset.h"
#include "ground.h"



class lighting {
public:
    glm::vec3 lightPosition, lightIntensity;
    float lightExposure;

    GLuint lightPositionID;
    GLuint lightIntensityID;
    GLuint lightSpaceMatrixID;
    GLuint lightExposureID;
    GLuint lightSpaceID;
    GLuint transformID;
    GLuint shadowFBO;
    GLuint shadowTexture;

    glm::mat4 lightSpaceMatrix;

    int shadowMapWidth, shadowMapHeight;

    GLuint programID, depthProgramID;

    bool saveDepth = true;
    lighting(GLuint programID, int shadowMapWidth, int shadowMapHeight);
    void setLightPosition(glm::vec3 lightPosition, glm::vec3 lightIntensity, float exposure);
    void shadowPass(glm::mat4 lightSpaceMatrix, std::vector<Asset> assets, std::vector<Plane> planes);
    void prepareLighting();
    void cleanup();
};


#endif //LIGHTING_H
