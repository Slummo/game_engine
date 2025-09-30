#pragma once

#include "systems/isystem.h"

class CollisionDetectionSystem : public ISystem {
public:
    void init(Engine& engine) override;
    void update(Engine& engine) override;
};
