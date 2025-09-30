#pragma once

#include "systems/isystem.h"

class RotationSystem : public ISystem {
public:
    void update(Engine& engine) override;
};
