#pragma once

#include "components/icomponent.h"

#include <string>

struct CollisionSound : public IComponent {
    CollisionSound(const std::string& filename) : sound_filename(std::string(filename)) {
    }

    std::string sound_filename;
    float min_impact_force = 0.0f;
};