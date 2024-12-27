#ifndef LIGHTING_H
#define LIGHTING_H
#include <memory>
#include <vector>
#include <glm/detail/type_mat.hpp>
#include <glm/detail/type_mat4x4.hpp>
#include <glm/detail/type_vec.hpp>
#include <glm/detail/type_vec3.hpp>

#include "asset.h"
#include "chunkLoader.h"
#include "cube.h"
#include "ground.h"
#define CHUNK_SIZE 512


struct LightSource {
    glm::vec3 position;
    glm::vec3 intensity;
    float exposure;
    glm::mat4 lightSpaceMatrix;
    GLuint shadowFBO;
};

class lighting {
public:
    std::vector<LightSource> lightSources;
    GLuint shadowMapArray;

    int shadowMapWidth, shadowMapHeight;

    GLuint programID, depthProgramID;

    lighting(GLuint programID, int shadowMapWidth, int shadowMapHeight);
    void setLightPosition(glm::vec3 lightPosition, glm::vec3 lightIntensity, float exposure);
    void pruneLights(int chunkX, int chunkZ);
    void add(glm::vec3 position, glm::vec3 intensity, float exposure);
    void shadowPass(glm::mat4 lightProjection, std::vector<Asset> assets,  std::vector<Cube> cubes, std::vector<Plane> planes);
    void prepareLighting();
    void cleanup();
};


#endif //LIGHTING_H
