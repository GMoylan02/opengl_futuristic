#ifndef TEXTURE_H
#define TEXTURE_H
#include <vector>

#include "../external/glad-3.3/include/glad/gl.h"
#include <tiny_gltf.h>

GLuint LoadTextureTileBox(const char *texture_file_path);
std::vector<GLuint> loadTextures(const tinygltf::Model &model);
#endif //TEXTURE_H
