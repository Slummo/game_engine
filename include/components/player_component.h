#pragma once

#include "components/icomponent.h"

#include <string>
#include <glm/glm.hpp>

struct PlayerComponent : public IComponent {
    PlayerComponent(std::string name = "unnamed_player") : name(name.empty() ? "unnamed_player" : std::move(name)) {
    }

    std::string name;
};