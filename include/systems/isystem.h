#pragma once

#include "core/ecs.h"
// So that every system has it
#include "components/components.h"

#include <string>
#include <cstdint>

class ISystem {
public:
    virtual ~ISystem() = default;

    // Called once when the engine initializes
    virtual void init(ECS&) {
    }

    // Called every variable-timestep frame
    virtual void update(ECS&, float /*dt*/) = 0;

    // Called on a fixed-timestep
    virtual void fixed_update(ECS&, float /*fixed_dt*/) {
    }

    // Called on shutdown
    virtual void shutdown(ECS&) {
    }
};
