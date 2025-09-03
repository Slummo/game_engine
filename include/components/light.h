#pragma once

#include "components/icomponent.h"

#include <glm/glm.hpp>

enum class LightType { Directional, Point, Spot };

struct Light : public IComponent {
    Light() = default;

    LightType type = LightType::Directional;
    glm::vec3 direction{0.0f, -1.0f, 0.0f};  // point down
    glm::vec3 color{1.0f};
    float intensity = 1.0f;
};
