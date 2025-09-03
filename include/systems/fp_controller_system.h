#pragma once

#include "systems/isystem.h"

class FirstPersonControllerSystem : public ISystem<InputContext, EventContext> {
public:
    void init(EntityManager& em, InputContext& ic, EventContext& ec) override;
    void update(EntityManager& em, InputContext& ic, EventContext& ec) override;
};
