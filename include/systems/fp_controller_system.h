#pragma once

#include "systems/isystem.h"
#include "managers/input_manager.h"

class FirstPersonControllerSystem : public ISystem {
public:
    FirstPersonControllerSystem(InputManager& in_mgr);

    void update(ECS& ecs, float dt) override;

private:
    InputManager& m_in_mgr;
};
