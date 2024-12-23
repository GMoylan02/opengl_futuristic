#include <cstdint>
#include <iostream>
#include <stb_image.h>
#include <tiny_gltf.h>
#include <texture.h>
#include <vector>

#include "../external/glad-3.3/include/glad/gl.h"

GLuint LoadTextureTileBox(const char *texture_file_path) {
    int w, h, channels;
    uint8_t* img = stbi_load(texture_file_path, &w, &h, &channels, 3);
    GLuint texture;
    // Generate an OpenGL texture and make use of it
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    // To tile textures on a box, we set wrapping to repeat
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

    if (img) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, w, h, 0, GL_RGB, GL_UNSIGNED_BYTE, img);
        glGenerateMipmap(GL_TEXTURE_2D);
    } else {
        std::cout << "Failed to load texture " << texture_file_path << std::endl;
    }
    stbi_image_free(img);

    return texture;
}
std::vector<GLuint> loadTextures(const tinygltf::Model &model) {
    std::vector<GLuint> textureIDs(model.textures.size(), 0);

    for (size_t i = 0; i < model.textures.size(); ++i) {
        const tinygltf::Texture &texture = model.textures[i];
        const tinygltf::Image &image = model.images[texture.source];

        if (image.uri.empty()) {
            std::cerr << "Texture " << i << " has no valid source image." << std::endl;
            continue;
        }

        // Generate a texture ID
        GLuint textureID;
        glGenTextures(1, &textureID);
        glBindTexture(GL_TEXTURE_2D, textureID);

        // Determine the format of the texture
        GLenum format = GL_RGB;
        if (image.component == 4) {
            format = GL_RGBA;
        } else if (image.component == 1) {
            format = GL_RED;
        }

        // Upload texture data to GPU
        glTexImage2D(GL_TEXTURE_2D, 0, format, image.width, image.height, 0, format, GL_UNSIGNED_BYTE, &image.image[0]);

        // Generate mipmaps for the texture
        glGenerateMipmap(GL_TEXTURE_2D);

        // Set texture parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

        // Store the texture ID
        textureIDs[i] = textureID;

        // Debug output
        std::cout << "Loaded texture " << i << " from " << image.uri << std::endl;
    }

    return textureIDs;
}

