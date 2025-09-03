#pragma once

#include "systems/isystem.h"

#include <glm/glm.hpp>
#include <vector>

class RigidBodySystem : public ISystem<PhysicsContext, EventContext> {
public:
    void init(EntityManager& em, PhysicsContext& pc, EventContext& ec) override;
    void update(EntityManager& em, PhysicsContext& pc, EventContext& ec) override;
};