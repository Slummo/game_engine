#pragma once

#include "systems/isystem.h"

class LightSystem : public ISystem<> {
public:
    void init(EntityManager& em) override;
    void update(EntityManager& /*em*/) override {
    }
};