//
// Created by eyeba on 17/12/2024.
//

#ifndef ASSET_H
#define ASSET_H
//GLuint LoadTextureTileBox(const char *texture_file_path);
extern glm::vec3 cameraPos;
extern glm::mat4 projectionMatrix;

#include "tiny_gltf.h"

GLuint loadTexture(const char* filename);

struct Asset {
    // Shader variable IDs
    GLuint mvpMatrixID;
    GLuint lightPositionID;
    GLuint lightIntensityID;
    GLuint programID;

    glm::vec3 position;
    glm::vec3 scale;

    tinygltf::Model model;

    struct PrimitiveObject {
        GLuint vao{};
        std::map<int, GLuint> vbos;
        GLsizei indexCount{};
        GLenum indexType{};
        GLuint materialIndex{}; // Optional, for material handling later
        GLuint textureID{};
    };
    std::vector<PrimitiveObject> primitiveObjects;

    glm::mat4 getNodeTransform(const tinygltf::Node &node);

    void computeGlobalNodeTransform(const tinygltf::Model &model,
                                    int nodeIndex,
                                    const glm::mat4 &parentTransform,
                                    std::vector<glm::mat4> &globalTransforms);

    bool loadModel(tinygltf::Model &model, const char *filename);

    void initialize(glm::vec3 position, glm::vec3 scale, const char* filename);

    void bindMesh(std::vector<PrimitiveObject> &primitiveObjects,
                  tinygltf::Model &model,
                  tinygltf::Mesh &mesh);

    std::vector<PrimitiveObject> bindModel(tinygltf::Model &model);

    void render(const glm::mat4& mvpMatrix/*, const glm::vec3& lightPosition, const glm::vec3& lightIntensity*/);
};

#endif //ASSET_H
