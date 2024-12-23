
#ifndef GROUND_H
#define GROUND_H

#include <glm/detail/type_mat.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include <glm/detail/type_vec.hpp>
#include <glm/detail/type_vec3.hpp>

#include "glad/gl.h"


class Plane {
public:
    glm::vec3 position;
    glm::vec3 scale;

    glm::mat4 modelMatrix;
    GLuint viewMatrixID;
    GLuint projectionMatrixID;
    GLfloat vertex_buffer_data[12] = {
        -0.5f, 0.0f, -0.5f, // bottom-left
         0.5f, 0.0f, -0.5f, // bottom-right
         0.5f, 0.0f,  0.5f, // top-right
        -0.5f, 0.0f,  0.5f  // top-left
    };

    GLfloat normal_buffer_data[12] = {
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f,
        0.0f, 1.0f, 0.0f
    };

    GLuint index_buffer_data[6] = {
        0, 2, 1,
        0, 3, 2
    };

    GLfloat uv_buffer_data[8] = {
        0.0f, 0.0f, // bottom-left
        4.0f, 0.0f, // bottom-right
        4.0f, 4.0f, // top-right
        0.0f, 4.0f  // top-left
    };

    // OpenGL buffers
    GLuint vertexArrayID;
    GLuint vertexBufferID;
    GLuint indexBufferID;
    GLuint normalBufferID;
    GLuint uvBufferID;
    GLuint textureID;
    GLuint modelMatrixID;
    GLuint baseColorFactorID;
    GLuint isLightID;

    // Shader variable IDs
    GLuint mvpMatrixID;
    GLuint textureSamplerID;
    GLuint programID;

    //todo tmp
    GLuint lightPositionID;
    GLuint lightIntensityID;
    glm::mat4 lightSpaceMatrix;
    GLuint numLights;
    Plane(glm::vec3 position, glm::vec3 scale);
    void render(glm::mat4 cameraMatrix);
    void renderDepth(GLuint programID, GLuint mvpMatrixID, const glm::mat4& lightSpaceMatrix);
    void cleanup();
};



#endif //GROUND_H
