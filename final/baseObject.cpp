#include "baseObject.h"

#include <glm/detail/type_vec.hpp>
#include <glm/detail/type_vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include "glad/gl.h"

BaseObject::BaseObject(GLuint programID, glm::vec3 position, glm::vec3 scale) {
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::scale(modelMatrix, scale);

    // Get a handle for our "MVP" uniforms
    cameraMatrixID = glGetUniformLocation(programID, "camera");
    transformMatrixID = glGetUniformLocation(programID, "transform");

    // Get a handle for our Model Matrix uniform
    modelMatrixID = glGetUniformLocation(programID, "modelMatrix");
    textureSamplerID = glGetUniformLocation(programID, "textureSampler");
    baseColorFactorID = glGetUniformLocation(programID, "baseColorFactor");
    isLightID = glGetUniformLocation(programID, "isLight");
    cameraPosID = glGetUniformLocation(programID, "cameraPos");
    this->programID = programID;
}



