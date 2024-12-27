
#ifndef GROUND_H
#define GROUND_H

#include <utility>
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
    GLuint cameraMatrixID;
    GLuint transformMatrixID;
    GLuint modelMatrixID;
    GLuint baseColorFactorID;
    GLuint isLightID;

    GLuint shininessID;
    GLuint cameraPosID;

    // Shader variable IDs
    GLuint textureSamplerID;
    GLuint programID;
    Plane(glm::vec3 position, glm::vec3 scale, const char *texture_file_path);
    void render(glm::mat4 cameraMatrix);
    void renderDepth(GLuint programID, GLuint lightMatID, GLuint tranMatID, const glm::mat4& lightSpaceMatrix);
    void cleanup();
    bool operator==(const Plane& other) const {
        return position == other.position && scale == other.scale;
    }

    // Friend declaration for PlaneHash
    friend struct PlaneHash;
};

struct PlaneHash {
    std::size_t operator()(const Plane& plane) const {
        std::size_t h1 = std::hash<float>()(plane.position.x) ^ std::hash<float>()(plane.position.y) ^ std::hash<float>()(plane.position.z);
        std::size_t h2 = std::hash<float>()(plane.scale.x) ^ std::hash<float>()(plane.scale.y) ^ std::hash<float>()(plane.scale.z);
        return h1 ^ (h2 << 1);
    }
};

struct PairHash {
    template <typename T1, typename T2>
    std::size_t operator()(const std::pair<T1, T2>& pair) const {
        auto hash1 = std::hash<T1>{}(pair.first);
        auto hash2 = std::hash<T2>{}(pair.second);
        // Combine the two hashes
        return hash1 ^ (hash2 << 1); // XOR with shifting
    }
};



#endif //GROUND_H
