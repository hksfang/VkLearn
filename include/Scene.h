#pragma once

#include <glm/glm.hpp>

struct SceneData {
    glm::mat4 view;
    glm::mat4 proj;
    glm::mat4 viewProj;
    glm::vec4 ambientColor;
    glm::vec4 sunlightDirection; // w for sun power
    glm::vec4 sunlightColor;
    glm::vec4 camPosition;
};
