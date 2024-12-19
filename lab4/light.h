//
// Created by eyeba on 19/12/2024.
//

#ifndef LIGHT_H
#define LIGHT_H
#include <glm/detail/type_vec.hpp>

struct Light {
    glm::vec3 lightPosition;
    glm::vec3 lightIntensity;

    Light() = default;

    Light(glm::vec3 lightIntensity, glm::vec3 lightPosition)
        : lightPosition(lightPosition), lightIntensity(lightIntensity) {}
};

//global vector of all light sources
extern std::vector<Light> lights;

#endif //LIGHT_H
