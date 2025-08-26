#pragma once

#include "systems/isystem.h"

class LightSystem : public ISystem {
public:
    void init(ECS& ecs) override;
    void update(ECS&, float /*dt*/) override {
    }
};