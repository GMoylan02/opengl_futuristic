
#ifndef CUBE_H
#define CUBE_H
#include <glm/detail/type_vec.hpp>
#include <glm/detail/type_vec3.hpp>
#include "baseObject.h"

#include "glad/gl.h"

class Cube : public BaseObject {
    public:
    glm::vec3 position;
    glm::vec3 scale;
    GLuint viewMatrixID;
    GLuint projectionMatrixID;
        GLfloat vertex_buffer_data[72] = {	// Vertex definition for a canonical box
            // Front face
            -1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,

            // Back face (-z)
            1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,

            // Left face
            -1.0f, -1.0f, -1.0f,
            -1.0f, -1.0f, 1.0f,
            -1.0f, 1.0f, 1.0f,
            -1.0f, 1.0f, -1.0f,

            // Right face
            1.0f, -1.0f, 1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, 1.0f, -1.0f,
            1.0f, 1.0f, 1.0f,

            // Top face
            -1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, 1.0f,
            1.0f, 1.0f, -1.0f,
            -1.0f, 1.0f, -1.0f,

            // Bottom face
            -1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, -1.0f,
            1.0f, -1.0f, 1.0f,
            -1.0f, -1.0f, 1.0f,
        };

        GLfloat color_buffer_data[72] = {
            // Front, red
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 0.0f,

            // Back, yellow
            1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 0.0f,

            // Left, green
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,
            0.0f, 1.0f, 0.0f,

            // Right, cyan
            0.0f, 1.0f, 1.0f,
            0.0f, 1.0f, 1.0f,
            0.0f, 1.0f, 1.0f,
            0.0f, 1.0f, 1.0f,

            // Top, blue
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,
            0.0f, 0.0f, 1.0f,

            // Bottom, magenta
            1.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 1.0f,
            1.0f, 0.0f, 1.0f,
        };

        GLuint index_buffer_data[36] = {		// 12 triangle faces of a box
            0, 1, 2,
            0, 2, 3,

            4, 5, 6,
            4, 6, 7,

            8, 9, 10,
            8, 10, 11,

            12, 13, 14,
            12, 14, 15,

            16, 17, 18,
            16, 18, 19,

            20, 21, 22,
            20, 22, 23,
        };

    GLfloat uv_buffer_data[48] = {
        // Front
        0.0f, 1.0f,
        1.0f, 1.0f,
        1.0f, 0.0f,
        0.0f, 0.0f,
        // Back
        0.0f, 1.0f,
        1.0f, 1.0f,
         1.0f, 0.0f,
         0.0f, 0.0f,
         // Left
         0.0f, 1.0f,
         1.0f, 1.0f,
         1.0f, 0.0f,
         0.0f, 0.0f,
         // Right
         0.0f, 1.0f,
         1.0f, 1.0f,
         1.0f, 0.0f,
         0.0f, 0.0f,
         // Top
        0.0f, 1.0f,
         1.0f, 1.0f,
         1.0f, 0.0f,
         0.0f, 0.0f,
         // Bottom
        0.0f, 1.0f,
         1.0f, 1.0f,
         1.0f, 0.0f,
         0.0f, 0.0f,
         };

    GLfloat normal_buffer_data[72] = {
        0.0f, 0.0f, 1.0f, // Normal for the first vertex
0.0f, 0.0f, 1.0f, // Normal for the second vertex
0.0f, 0.0f, 1.0f, // Normal for the third vertex
0.0f, 0.0f, 1.0f, // Normal for the fourth vertex

        // Back face (z = -1)
0.0f, 0.0f, -1.0f, // Normal for the first vertex
0.0f, 0.0f, -1.0f, // Normal for the second vertex
0.0f, 0.0f, -1.0f, // Normal for the third vertex
0.0f, 0.0f, -1.0f, // Normal for the fourth vertex

        // Left face (x = -1)
        -1.0f, 0.0f, 0.0f, // Normal for the first vertex
        -1.0f, 0.0f, 0.0f, // Normal for the second vertex
        -1.0f, 0.0f, 0.0f, // Normal for the third vertex
        -1.0f, 0.0f, 0.0f, // Normal for the fourth vertex

        // Right face (x = 1)
        1.0f, 0.0f, 0.0f, // Normal for the first vertex
        1.0f, 0.0f, 0.0f, // Normal for the second vertex
        1.0f, 0.0f, 0.0f, // Normal for the third vertex
        1.0f, 0.0f, 0.0f, // Normal for the fourth vertex

        // Top face (y = 1)
        0.0f, 1.0f, 0.0f, // Normal for the first vertex
        0.0f, 1.0f, 0.0f, // Normal for the second vertex
        0.0f, 1.0f, 0.0f, // Normal for the third vertex
        0.0f, 1.0f, 0.0f, // Normal for the fourth vertex

        // Bottom face (y = -1)
        0.0f, -1.0f, 0.0f, // Normal for the first vertex
        0.0f, -1.0f, 0.0f, // Normal for the second vertex
        0.0f, -1.0f, 0.0f, // Normal for the third vertex
        0.0f, -1.0f, 0.0f  // Normal for the fourth vertex
    };
    // OpenGL buffers
    GLuint vertexArrayID;
    GLuint vertexBufferID;
    GLuint indexBufferID;
    GLuint normalBufferID;
    GLuint uvBufferID;
    GLuint textureID;
    GLuint shininessID;

    Cube(GLuint programID, glm::vec3 position, glm::vec3 scale, const char *texture_file_path);
    void render(glm::mat4 cameraMatrix);
    void renderDepth(GLuint programID, GLuint lightMatID, GLuint tranMatID, const glm::mat4& lightSpaceMatrix);
    void cleanup();
};

#endif //CUBE_H