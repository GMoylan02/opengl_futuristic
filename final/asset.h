#ifndef ASSET_H
#define ASSET_H
#include "glad/gl.h"
#include "baseObject.h"
//GLuint LoadTextureTileBox(const char *texture_file_path);
extern glm::vec3 cameraPos;
extern glm::mat4 projectionMatrix;

#include "texture.h"


class Asset : public BaseObject {
public:
    GLuint jointMatricesID;
    tinygltf::Model model;

    struct PrimitiveObject {
        GLuint vao;
        std::map<int, GLuint> vbos;
        int indexCount;
        GLenum indexType;
        GLuint textureID;
        glm::vec4 baseColorFactor;
        bool isLight;
        GLuint shininessID;
        int shininess;
    };
    std::vector<PrimitiveObject> primitiveObjects;

    glm::mat4 getNodeTransform(const tinygltf::Node &node);

    void computeGlobalNodeTransform(const tinygltf::Model &model,
                                    int nodeIndex,
                                    const glm::mat4 &parentTransform,
                                    std::vector<glm::mat4> &globalTransforms);

    bool loadModel(tinygltf::Model &model, const char *filename);

    Asset(GLuint programID, glm::vec3 translation, glm::vec3 scale, const char * filepath);

    void bindMesh(std::vector<PrimitiveObject> &primitiveObjects,
                  tinygltf::Model &model,
                  tinygltf::Mesh &mesh);

    std::vector<PrimitiveObject> bindModel(tinygltf::Model &model);

    void render(const glm::mat4& mvpMatrix/*, const glm::vec3& lightPosition, const glm::vec3& lightIntensity*/);
    void renderDepth(GLuint programID, GLuint lightMatID, GLuint tranMatID, const glm::mat4& lightSpaceMatrix) override;
};

#endif //ASSET_H
