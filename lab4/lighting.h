

#ifndef LIGHTING_H
#define LIGHTING_H
#include <glm/detail/type_mat.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include <glm/detail/type_vec.hpp>
#include <glm/detail/type_vec3.hpp>

#include "asset.h"
#include "cube.h"
#include "ground.h"
#include "glad/gl.h"


class lighting {
public:
    GLuint programID;
    GLuint depthProgramID;
    glm::vec3 lightPosition, lightIntensity;
    float lightExposure;
    int shadowMapWidth;
    int shadowMapHeight;
    GLuint lightPositionID;
    GLuint lightIntensityID;
    GLuint lightSpaceMatrixID;
    GLuint lightExposureID;
    GLuint depthMatrixID;
    GLuint shadowFBO;
    GLuint shadowTexture;
    glm::mat4 lightSpaceMatrix;
    bool saveDepth = true;
    void initialize(GLuint programID, int shadowMapWidth, int shadowMapHeight);
    void shadowPass(glm::mat4 lightSpaceMatrix, GLuint vertexBufferID, GLuint indexBufferID, int indexCount);
    void shadowPass(glm::mat4 lightSpaceMatrix, std::vector<Asset> assets/*todo update for all objects*/, std::vector<Plane> planes);
    void lightSetup();
    void setLightPosition(glm::vec3 lightPosition, glm::vec3 lightIntensity, float exposure);
};



#endif //LIGHTING_H
