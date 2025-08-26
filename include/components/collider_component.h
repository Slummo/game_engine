#pragma once

#include "core/types.h"
#include "components/icomponent.h"

#include <glm/glm.hpp>
#include <string>
#include <cstdint>
#include <algorithm>

enum class HitboxType { OBB, Sphere, Capsule };  // TODO implement capsule collision
using LayerMask = uint32_t;

struct ColliderComponent : public IComponent {
    ColliderComponent(HitboxType type = HitboxType::OBB) : type(type) {
    }

    glm::vec3 size{1.0f};  // dimensions for OBB, x radius for sphere, x radius y height for capsule
    HitboxType type = HitboxType::OBB;
    glm::vec3 offset{0.0f};   // local offset from the entity's origin
    bool is_trigger = false;  // trigger or physical collider
    bool is_enabled = true;
    LayerMask layer = 1;
    LayerMask collides_with = 0xFFFFFFFF;
};