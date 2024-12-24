#include <algorithm>
#include <tiny_gltf.h>

#include <render/shader.h>

#include <vector>
#include <iostream>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <glm/gtx/string_cast.hpp>
#include <skybox.h>
#include <iomanip>
#include <math.h>
#include <light.h>
#include <asset.h>

#include <cube.h>
#include <stb_image.h>

#include "texture.h"

#define BUFFER_OFFSET(i) ((char *)NULL + (i))


glm::mat4 Asset::getNodeTransform(const tinygltf::Node& node) {
    glm::mat4 transform(1.0f);

    if (node.matrix.size() == 16) {
        transform = glm::make_mat4(node.matrix.data());
    } else {
        if (node.translation.size() == 3) {
            transform = glm::translate(transform, glm::vec3(node.translation[0], node.translation[1], node.translation[2]));
        }
        if (node.rotation.size() == 4) {
            glm::quat q(node.rotation[3], node.rotation[0], node.rotation[1], node.rotation[2]);
            transform *= glm::mat4_cast(q);
        }
        if (node.scale.size() == 3) {
            transform = glm::scale(transform, glm::vec3(node.scale[0], node.scale[1], node.scale[2]));
        }
    }
    return transform;
}

void Asset::computeGlobalNodeTransform(const tinygltf::Model& model,
                                int nodeIndex,
                                const glm::mat4& parentTransform,
                                std::vector<glm::mat4>& globalTransforms) {
    const tinygltf::Node& node = model.nodes[nodeIndex];
    glm::mat4 currentTransform = parentTransform * getNodeTransform(node);

    globalTransforms[nodeIndex] = currentTransform;

    for (int childIndex : node.children) {
        computeGlobalNodeTransform(model, childIndex, currentTransform, globalTransforms);
    }
}

bool Asset::loadModel(tinygltf::Model& model, const char* filename) {
    tinygltf::TinyGLTF loader;
    std::string err;
    std::string warn;

    bool res = loader.LoadASCIIFromFile(&model, &err, &warn, filename);
    if (!warn.empty()) {
        std::cout << "WARN: " << warn << std::endl;
    }
    if (!err.empty()) {
        std::cout << "ERR: " << err << std::endl;
    }
    if (!res) {
        std::cout << "Failed to load glTF: " << filename << std::endl;
    } else {
        std::cout << "Loaded glTF: " << filename << std::endl;
    }

    return res;
}

void Asset::initialize(GLuint programID, glm::vec3 position, glm::vec3 scale, const char* filename) {
    // Modify your path if needed
    if (!loadModel(model, filename)) {
        return;
    }
    // Scale and translate the model
    modelMatrix = glm::mat4(1.0f);
    modelMatrix = glm::translate(modelMatrix, position);
    modelMatrix = glm::scale(modelMatrix, scale);

    // Prepare buffers for rendering
    primitiveObjects = bindModel(model);

    this->programID = programID;

    // Get a handle for GLSL variables
    cameraMatrixID = glGetUniformLocation(programID, "camera");
    transformMatrixID = glGetUniformLocation(programID, "transform");
    modelMatrixID = glGetUniformLocation(programID, "modelMatrix");
    textureSamplerID = glGetUniformLocation(programID, "textureSampler");
    baseColorFactorID = glGetUniformLocation(programID, "baseColorFactor");
    isLightID = glGetUniformLocation(programID, "isLight");
    cameraPosID = glGetUniformLocation(programID, "cameraPos");

    for (auto prim : primitiveObjects) {
        prim.shininessID = glGetUniformLocation(programID, "shininess");
    }

}

void Asset::bindMesh(std::vector<PrimitiveObject>& primitiveObjects,
              tinygltf::Model& model,
              tinygltf::Mesh& mesh) {
    std::map<int, GLuint> vbos;
    for (size_t i = 0; i < model.bufferViews.size(); ++i) {
        const tinygltf::BufferView& bufferView = model.bufferViews[i];

        if (bufferView.target == 0) {
            continue;
        }

        const tinygltf::Buffer& buffer = model.buffers[bufferView.buffer];
        GLuint vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(bufferView.target, vbo);
        glBufferData(bufferView.target, bufferView.byteLength,
                     &buffer.data.at(0) + bufferView.byteOffset, GL_STATIC_DRAW);
        vbos[i] = vbo;
    }

    for (size_t i = 0; i < mesh.primitives.size(); ++i) {
        tinygltf::Primitive primitive = mesh.primitives[i];

        GLuint vao;
        glGenVertexArrays(1, &vao);
        glBindVertexArray(vao);

        for (auto& attrib : primitive.attributes) {
            const tinygltf::Accessor& accessor = model.accessors[attrib.second];
            const tinygltf::BufferView& bufferView = model.bufferViews[accessor.bufferView];
            glBindBuffer(GL_ARRAY_BUFFER, vbos[accessor.bufferView]);

            int size = accessor.type == TINYGLTF_TYPE_SCALAR ? 1 : accessor.type;
            int vaa = -1;
            if (attrib.first == "POSITION") vaa = 0;
            if (attrib.first == "NORMAL") vaa = 1;
            if (attrib.first == "TEXCOORD_0") vaa = 2;

            if (vaa > -1) {
                glEnableVertexAttribArray(vaa);
                glVertexAttribPointer(vaa, size, accessor.componentType,
                                      accessor.normalized ? GL_TRUE : GL_FALSE,
                                      accessor.ByteStride(bufferView),
                                      BUFFER_OFFSET(accessor.byteOffset));
            } else {
                std::cout << "vaa missing: " << attrib.first << std::endl;
            }
        }

        PrimitiveObject primitiveObject;
        primitiveObject.vao = vao;
        primitiveObject.vbos = vbos;
        primitiveObjects.push_back(primitiveObject);

        glBindVertexArray(0);
    }
}

std::vector<Asset::PrimitiveObject> Asset::bindModel(tinygltf::Model &model) {
    std::vector<PrimitiveObject> primitives;

    // Load all textures
    auto textureIDs = loadTextures(model);

    // Create VBOs for all buffer views
    std::map<int, GLuint> bufferViewVBOs;
    for (size_t i = 0; i < model.bufferViews.size(); ++i) {
        const tinygltf::BufferView &bufferView = model.bufferViews[i];
        const tinygltf::Buffer &buffer = model.buffers[bufferView.buffer];

        GLuint vbo;
        glGenBuffers(1, &vbo);
        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, bufferView.byteLength,
                     &buffer.data[bufferView.byteOffset], GL_STATIC_DRAW);

        bufferViewVBOs[i] = vbo;
    }

    // Iterate through all meshes and primitives
    for (const auto &mesh : model.meshes) {
        for (const auto &primitive : mesh.primitives) {
            PrimitiveObject primitiveObject;

            // Create a VAO for the primitive
            glGenVertexArrays(1, &primitiveObject.vao);
            glBindVertexArray(primitiveObject.vao);

            // Set up attributes (POSITION, TEXCOORD_0, NORMAL)
            for (const auto &attrib : primitive.attributes) {
                const std::string &attribName = attrib.first;
                const tinygltf::Accessor &accessor = model.accessors[attrib.second];
                const tinygltf::BufferView &bufferView = model.bufferViews[accessor.bufferView];

                int size = 1;
                if (accessor.type != TINYGLTF_TYPE_SCALAR) {
                    size = accessor.type;
                }

                GLuint vbo = bufferViewVBOs[accessor.bufferView];
                glBindBuffer(GL_ARRAY_BUFFER, vbo);

                int location = -1;
                if (attribName == "POSITION") location = 0;
                if (attribName == "NORMAL") location = 1;
                if (attribName == "TEXCOORD_0") location = 2;

                if (location != -1) {
                    glEnableVertexAttribArray(location);
                    glVertexAttribPointer(location, size, accessor.componentType,
                                          accessor.normalized ? GL_TRUE : GL_FALSE,
                                          accessor.ByteStride(bufferView),
                                          (void *)(accessor.byteOffset));
                }
            }

            // Set up the element array buffer (indices)
            if (primitive.indices >= 0) {
                const tinygltf::Accessor &indexAccessor = model.accessors[primitive.indices];
                const tinygltf::BufferView &indexBufferView = model.bufferViews[indexAccessor.bufferView];

                GLuint ebo;
                glGenBuffers(1, &ebo);
                glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
                glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexBufferView.byteLength,
                             &model.buffers[indexBufferView.buffer].data[indexBufferView.byteOffset], GL_STATIC_DRAW);

                primitiveObject.indexCount = indexAccessor.count;
                primitiveObject.indexType = indexAccessor.componentType;
            }


            // Bind texture to the primitive
            if (primitive.material >= 0) {
                const tinygltf::Material &material = model.materials[primitive.material];
                if (material.pbrMetallicRoughness.baseColorTexture.index >= 0) {
                    int textureIndex = material.pbrMetallicRoughness.baseColorTexture.index;
                    primitiveObject.textureID = textureIDs[textureIndex];
                }
                else {
                    primitiveObject.textureID = 0;
                }
                // Extract baseColorFactor
                if (material.pbrMetallicRoughness.baseColorFactor.size() == 4) {
                    primitiveObject.baseColorFactor = glm::vec4(
                        material.pbrMetallicRoughness.baseColorFactor[0],
                        material.pbrMetallicRoughness.baseColorFactor[1],
                        material.pbrMetallicRoughness.baseColorFactor[2],
                        material.pbrMetallicRoughness.baseColorFactor[3]
                    );
                }
                else {
                    primitiveObject.baseColorFactor = glm::vec4(1.0f); // Default to opaque white
                }
                // Extract roughness and calculate shininess
                float roughness = material.pbrMetallicRoughness.roughnessFactor;
                roughness = glm::clamp(roughness, 0.0f, 1.0f); // Ensure roughness is in [0, 1]

                // Map roughness to shininess (Phong model)
                primitiveObject.shininess = 1.0f / glm::pow(roughness + 0.01f, 2.0f);

                // glossiness currently goes unused
                if (material.extensions.find("KHR_materials_pbrSpecularGlossiness") != material.extensions.end()) {
                    const auto &extension = material.extensions.at("KHR_materials_pbrSpecularGlossiness");
                    float glossiness = 1.0f; // Default glossiness
                    if (extension.Has("glossinessFactor") && extension.Get("glossinessFactor").IsNumber()) {
                        glossiness = static_cast<float>(extension.Get("glossinessFactor").Get<double>());
                    }

                    glossiness = glm::clamp(glossiness, 0.0f, 1.0f);

                    // Map glossiness to shininess
                    primitiveObject.shininess = glossiness * 256.0f; // Scale shininess for glossiness
                }
                primitiveObject.isLight = false;
            }
            else {
                primitiveObject.textureID = 0;
                primitiveObject.baseColorFactor = glm::vec4(1.0f); // Default to opaque white
            }


            glBindVertexArray(0);
            primitives.push_back(primitiveObject);
        }
    }
    return primitives;
}

void Asset::render(const glm::mat4& cameraMatrix/*, const glm::vec3& lightPosition, const glm::vec3& lightIntensity*/) {
 glUseProgram(programID);
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

        // Pass in model-view-projection matrix
        glUniformMatrix4fv(cameraMatrixID, 1, GL_FALSE, &cameraMatrix[0][0]);
        glUniformMatrix4fv(transformMatrixID, 1, GL_FALSE, &modelMatrix[0][0]);
        glUniformMatrix4fv(modelMatrixID, 1, GL_FALSE, &modelMatrix[0][0]);

        // Separate opaque and transparent objects
        std::vector<const PrimitiveObject*> opaqueObjects;
        std::vector<const PrimitiveObject*> transparentObjects;

        for (const auto& primitive : primitiveObjects) {
            if (primitive.baseColorFactor.a < 1.0f) {
                transparentObjects.push_back(&primitive);
            }
            else {
                opaqueObjects.push_back(&primitive);
            }
        }

        // Render opaque objects first
        for (const auto* primitive : opaqueObjects) {
            glBindVertexArray(primitive->vao);

            if (primitive->textureID) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, primitive->textureID);
                glUniform1i(textureSamplerID, 0);
            }

            glUniform1i(isLightID, primitive->isLight ? 1 : 0);
            glUniform1i(primitive->shininessID, primitive->shininess);
            glUniform3fv(cameraPosID, 1, &cameraPos[0]);
            glUniform4fv(baseColorFactorID, 1, &primitive->baseColorFactor[0]);
            glDrawElements(GL_TRIANGLES, primitive->indexCount, primitive->indexType, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        // Sort transparent objects by distance from the camera
        std::sort(transparentObjects.begin(), transparentObjects.end(),
            [&cameraMatrix](const PrimitiveObject* a, const PrimitiveObject* b) {
                glm::vec3 aPos = glm::vec3(cameraMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)); // Center of object A
                glm::vec3 bPos = glm::vec3(cameraMatrix * glm::vec4(0.0f, 0.0f, 0.0f, 1.0f)); // Center of object B
                return glm::length(aPos) > glm::length(bPos); // Sort back-to-front
            });

        // Render transparent objects
        for (const auto* primitive : transparentObjects) {
            glBindVertexArray(primitive->vao);

            if (primitive->textureID) {
                glActiveTexture(GL_TEXTURE0);
                glBindTexture(GL_TEXTURE_2D, primitive->textureID);
                glUniform1i(textureSamplerID, 0);
            }
            glUniform1i(isLightID, primitive->isLight ? 1 : 0);
            glUniform1i(primitive->shininessID, primitive->shininess);
            glUniform3fv(cameraPosID, 1, glm::value_ptr(cameraPos));
            glUniform4fv(baseColorFactorID, 1, &primitive->baseColorFactor[0]);
            glDrawElements(GL_TRIANGLES, primitive->indexCount, primitive->indexType, 0);
            glBindTexture(GL_TEXTURE_2D, 0);
        }

        // Reset state
        glDisable(GL_BLEND);
        glUseProgram(0);
        glBindVertexArray(0);
}

void Asset::renderDepth(GLuint programID, GLuint lightMatID, GLuint tranMatID, const glm::mat4& lightSpaceMatrix) {
    glUseProgram(programID);

    // Pass the MVP matrix to the shader
    glUniformMatrix4fv(lightMatID, 1, GL_FALSE, &lightSpaceMatrix[0][0]);
    glUniformMatrix4fv(tranMatID, 1, GL_FALSE, &modelMatrix[0][0]);

    // Render each primitive
    for (const auto& primitive : primitiveObjects) {
        glBindVertexArray(primitive.vao);
        glDrawElements(GL_TRIANGLES, primitive.indexCount, primitive.indexType, 0);
    }

    // Reset state
    glBindVertexArray(0);
    glUseProgram(0);
}


