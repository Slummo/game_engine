#pragma once

#include "core/types/id.h"

#include <glm/glm.hpp>

struct Contact {
    EntityID a;
    EntityID b;
    float penetration = 0.0f;
    glm::vec3 position{0.0f};
    glm::vec3 normal{0.0f};  // points from A to B
    bool is_trigger = false;
};