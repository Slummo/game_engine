#pragma once

#include "systems/isystem.h"

class CameraSystem : public ISystem {
public:
    void update(Engine& engine) override;
};