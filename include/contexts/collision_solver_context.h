#pragma once

#include "contexts/icontext.h"
#include "core/types.h"

#include <vector>
#include <glm/glm.hpp>

struct Contact {
    EntityID a = 0;
    EntityID b = 0;
    float penetration = 0.0f;
    glm::vec3 position{0.0f};
    glm::vec3 normal{0.0f};  // points from A to B
    bool is_trigger = false;
};

struct CollisionContext : public IContext {
    std::vector<Contact> contacts;
};