#pragma once

#include "systems/isystem.h"

#include <glm/glm.hpp>
#include <vector>

class RigidBodySystem : public ISystem<PhysicsContext> {
public:
    void update(EntityManager& em, PhysicsContext& pc) override;
};