#pragma once

#include "systems/isystem.h"

#include <glm/glm.hpp>
#include <vector>

class RigidBodySystem : public ISystem {
public:
    RigidBodySystem(const glm::vec3& gravity = glm::vec3(0.0f, -9.81f, 0.0f));

    void update(ECS& /*ecs*/, float /*dt*/) override;
    void fixed_update(ECS& ecs, float fixed_dt) override;

private:
    glm::vec3 m_gravity;
};