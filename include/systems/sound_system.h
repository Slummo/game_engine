#pragma once

#include "systems/isystem.h"

class SoundSystem : public ISystem<> {
public:
    void update(EntityManager& em) override;
};