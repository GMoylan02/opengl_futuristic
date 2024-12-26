#include <iostream>

#include "glad/gl.h"
#include <light.h>
#include <ostream>
#include <vector>
#include <glm/detail/type_mat.hpp>
#include <glm/detail/type_vec3.hpp>
#include <glm/gtc/matrix_transform.hpp>

void passLightsToShader(GLuint programID) {

    int numLights = lights.size();

    glUniform1i(glGetUniformLocation(programID, "numLights"), numLights);

    // Upload light positions and intensities only if numLights > 0
    if (numLights > 0) {
        std::vector<glm::vec3> lightPositions, lightIntensities;
        for (const auto& light : lights) {
            lightPositions.push_back(light.lightPosition);
            lightIntensities.push_back(light.lightIntensity);
        }
        glUniform3fv(glGetUniformLocation(programID, "lightPositions"), numLights, &lightPositions[0][0]);
        glUniform3fv(glGetUniformLocation(programID, "lightIntensities"), numLights, &lightIntensities[0][0]);
    }
}
