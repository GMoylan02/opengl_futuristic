#ifndef INSTANCE_H
#define INSTANCE_H

#include "asset.h"


class Instance {
public:
    glm::mat4 modelMatrix;

    Instance(const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotation) {
        setTransform(position, scale, rotation);
    }

    void setTransform(const glm::vec3& position, const glm::vec3& scale, const glm::vec3& rotation) {
        glm::mat4 trans = glm::translate(glm::mat4(1.0f), position);
        glm::mat4 rot = glm::rotate(glm::mat4(1.0f), rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
        rot = glm::rotate(rot, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
        rot = glm::rotate(rot, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));
        glm::mat4 scl = glm::scale(glm::mat4(1.0f), scale);

        modelMatrix = trans * rot * scl;
    }
};



#endif //INSTANCE_H
