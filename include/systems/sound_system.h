#pragma once

#include "systems/isystem.h"
#include "managers/asset_manager.h"

class SoundSystem : public ISystem<> {
public:
    void init(EntityManager& em) override;
    void update(EntityManager& em) override;
};