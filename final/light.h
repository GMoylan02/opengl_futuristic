#ifndef LIGHT_H
#define LIGHT_H
#include <vector>
#include <glm/detail/type_vec.hpp>
#include <glm/detail/type_vec3.hpp>

struct Light {
    glm::vec3 lightPosition;
    glm::vec3 lightIntensity;

    Light() = default;

    Light(glm::vec3 lightIntensity, glm::vec3 lightPosition)
        : lightPosition(lightPosition), lightIntensity(lightIntensity) {}
};

void passLightsToShader(GLuint programID);

//global vector of all light sources
extern std::vector<Light> lights;

#endif //LIGHT_H
