#pragma once

#include "systems/isystem.h"

class RotationSystem : public ISystem {
public:
    RotationSystem() = default;

    void update(ECS& ecs, float dt) override;
};
