#pragma once

#include "systems/isystem.h"

class FirstPersonControllerSystem : public ISystem<PhysicsContext, InputContext> {
public:
    void update(EntityManager& em, PhysicsContext& pc, InputContext& ic) override;
};
