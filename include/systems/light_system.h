#pragma once

#include "systems/isystem.h"

class LightSystem : public ISystem {
public:
    void init(Engine& engine) override;
    void update(Engine& /*engine*/) override {
    }
};