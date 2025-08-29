#pragma once

#include "systems/isystem.h"

class SoundSystem : public ISystem {
public:
    void update(ECS& ecs, float dt) override;
};