#pragma once

#include "systems/isystem.h"
#include "managers/asset_manager.h"

class SoundSystem : public ISystem<EventContext> {
public:
    void init(EntityManager& em, EventContext& ec) override;
    void update(EntityManager& em, EventContext& ec) override;
};