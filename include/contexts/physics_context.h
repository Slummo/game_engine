#pragma once

#include "contexts/icontext.h"

#include <glm/glm.hpp>

struct PhysicsContext : public IContext {
    glm::vec3 gravity;
    float dt = 0.0f;

    PhysicsContext(const glm::vec3& gravity = glm::vec3(0.0f, -9.81f, 0.0f)) : gravity(gravity) {
    }
};