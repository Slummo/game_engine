#pragma once

#include "components/icomponent.h"

#include <glm/glm.hpp>

struct Rotator : public IComponent {
    Rotator(float speed_deg = 45.0f, glm::vec3 axis = glm::vec3(0.0f, 1.0f, 0.0f)) : speed_deg(speed_deg), axis(axis) {
    }

    float speed_deg = 45.0f;           // degrees per second
    glm::vec3 axis{0.0f, 1.0f, 0.0f};  // default rotate around Y
};
