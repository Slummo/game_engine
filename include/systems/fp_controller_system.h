#pragma once

#include "systems/isystem.h"

class FirstPersonControllerSystem : public ISystem<PhysicsContext, InputContext> {
public:
    void init(EntityManager& em, PhysicsContext& pc, InputContext& ic) override;
    void update(EntityManager& em, PhysicsContext& pc, InputContext& ic) override;
};
