/*
#ifndef BASEOBJECT_H
#define BASEOBJECT_H
#include <glm/detail/type_mat.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include <glm/detail/type_vec.hpp>

#include "glad/gl.h"

class BaseObject {
public:
    virtual ~BaseObject() = default;
    GLuint cameraMatrixID;
    GLuint transformMatrixID;
    GLuint programID;
    GLuint textureSamplerID;
    GLuint modelMatrixID;
    GLuint baseColorFactorID;
    GLuint isLightID;
    glm::mat4 modelMatrix;
    GLuint cameraPosID;
    BaseObject(GLuint programID, glm::vec3 position, glm::vec3 scale);
    virtual void renderDepth(GLuint programID, GLuint lightMatID, const glm::mat4& lightSpaceMatrix) = 0;
};

#endif //BASEOBJECT_H
*/