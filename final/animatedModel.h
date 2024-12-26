#ifndef ANIMATEDMODEL_H
#define ANIMATEDMODEL_H
#define BUFFER_OFFSET(i) ((char *)NULL + (i))
#include <tiny_gltf.h>
#include <glm/detail/type_mat.hpp>
#include <glm/detail/type_vec.hpp>

#include "glad/gl.h"


struct AnimatedModel {
    GLuint mvpMatrixID;
    GLuint jointMatricesID;
    GLuint lightPositionID;
    GLuint lightIntensityID;
    GLuint programID;
    GLuint jointIndicesID;
    GLuint jointWeightsID;

    tinygltf::Model model;
    struct PrimitiveObject {
        GLuint vao;
        std::map<int, GLuint> vbos;
    };
    std::vector<PrimitiveObject> primitiveObjects;
    struct SkinObject {
        // Transforms the geometry into the space of the respective joint
        std::vector<glm::mat4> inverseBindMatrices;

        // Transforms the geometry following the movement of the joints
        std::vector<glm::mat4> globalJointTransforms;

        // Combined transforms
        std::vector<glm::mat4> jointMatrices;
    };
    std::vector<SkinObject> skinObjects;

    // Animation
    struct SamplerObject {
        std::vector<float> input;
        std::vector<glm::vec4> output;
        int interpolation;
    };
    struct ChannelObject {
        int sampler;
        std::string targetPath;
        int targetNode;
    };
    struct AnimationObject {
        std::vector<SamplerObject> samplers;	// Animation data
    };
    std::vector<AnimationObject> animationObjects;
    glm::mat4 getNodeTransform(const tinygltf::Node& node);
    void computeLocalNodeTransform(const tinygltf::Model& model,
        int nodeIndex,
        std::vector<glm::mat4> &localTransforms);
    void computeGlobalNodeTransform(const tinygltf::Model& model,
        const std::vector<glm::mat4> &localTransforms,
        int nodeIndex, const glm::mat4& parentTransform,
        std::vector<glm::mat4> &globalTransforms);
    std::vector<SkinObject> prepareSkinning(const tinygltf::Model &model);
    int findKeyframeIndex(const std::vector<float>& times, float animationTime);
    std::vector<AnimationObject> prepareAnimation(const tinygltf::Model &model);
    void updateAnimation(
        const tinygltf::Model &model,
        const tinygltf::Animation &anim,
        const AnimationObject &animationObject,
        float time,
        std::vector<glm::mat4> &nodeTransforms);
    void updateSkinning(const std::vector<glm::mat4> &nodeTransforms);
    void update(float time);
    bool loadModel(tinygltf::Model &model, const char *filename);
    void initialize();
    void bindMesh(std::vector<PrimitiveObject> &primitiveObjects,
                tinygltf::Model &model, tinygltf::Mesh &mesh);
    void bindModelNodes(std::vector<PrimitiveObject> &primitiveObjects,
                        tinygltf::Model &model,
                        tinygltf::Node &node);
    std::vector<PrimitiveObject> bindModel(tinygltf::Model &model);
    void drawMesh(const std::vector<PrimitiveObject> &primitiveObjects,
                tinygltf::Model &model, tinygltf::Mesh &mesh);
    void drawModelNodes(const std::vector<PrimitiveObject>& primitiveObjects,
                        tinygltf::Model &model, tinygltf::Node &node);
    void drawModel(const std::vector<PrimitiveObject>& primitiveObjects,
                tinygltf::Model &model);
    void render(glm::mat4 cameraMatrix);
    void printJointMatrices(const std::vector<glm::mat4>& jointMatrices);
    void cleanup();
};

#endif //ANIMATEDMODEL_H
