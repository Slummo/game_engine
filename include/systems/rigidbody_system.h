#pragma once

#include "systems/isystem.h"

#include <glm/glm.hpp>
#include <vector>

class RigidBodySystem : public ISystem {
public:
    void init(Engine& engine) override;
    void update(Engine& engine) override;
};