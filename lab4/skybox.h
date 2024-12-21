//
// Created by eyeba on 16/12/2024.
//

#ifndef SKYBOX_H
#define SKYBOX_H
#include <glm/detail/type_vec.hpp>
#include <glm/detail/type_vec3.hpp>

#include "glad/gl.h"

GLuint LoadTextureTileBox(const char *texture_file_path);

struct SkyBox {
    glm::vec3 position;		// Position of the box
    glm::vec3 scale;		// Size of the box in each axis
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
		0, 2,1,
		0, 3,2,

		4, 6,5,
		4, 7,6,

		8, 10,9,
		8, 11,10,

		12, 14,13,
		12, 15,14,

		16, 18,17,
		16, 19,18,

		20, 22,21,
		20, 23,22
	};

		GLfloat uv_buffer_data[48] = {
			// Front (+Z)
			0.5f, 0.666f,
			0.25f, 0.666f,
			0.25f, 0.333f,
			0.5f, 0.333f,

			// Back (-Z)
			1.0f, 0.665f,
			0.75f, 0.665f,
			0.75f, 0.334f,
			1.0f, 0.334f,

			// Left (+X)
			0.75f, 0.665f,
			0.5f, 0.665f,
			0.5f, 0.334f,
			0.75f, 0.334f,

			// Right (-X)
			0.25f, 0.665f,
			0.0f, 0.665f,
			0.0f, 0.334f,
			0.25f, 0.334f,

			// Top (+Y)
			0.499f, 0.333f,
			0.251f, 0.333f,
			0.251f, 0.0f,
			0.499f, 0.0f,

			// Bottom (-Y)
			0.499f, 1.0,
			0.251f, 1.0f,
			0.251f, 0.666f,
			0.499f, 0.666f
		};
    GLuint vertexArrayID;
    GLuint vertexBufferID;
    GLuint indexBufferID;
    GLuint colorBufferID;
    GLuint uvBufferID;
    GLuint textureID;
    GLuint mvpMatrixID;
    GLuint textureSamplerID;
    GLuint skybox_program_id;
    void initialize(glm::vec3 position, glm::vec3 scale);
    void render(glm::mat4 cameraMatrix);
    void cleanup();
};


#endif //SKYBOX_H