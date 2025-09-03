#pragma once

#include "core/types.h"
#include "components/icomponent.h"

#include <glm/glm.hpp>
#include <string>
#include <cstdint>
#include <algorithm>

enum class ColliderType { OBB, Sphere, Capsule };  // TODO implement capsule collision
using LayerMask = uint32_t;

struct Collider : public IComponent {
    Collider(ColliderType type = ColliderType::OBB) : type(type) {
    }

    ColliderType type = ColliderType::OBB;
    glm::vec3 size{1.0f};     // dimensions for OBB, x radius for sphere, x radius y height for capsule
    glm::vec3 offset{0.0f};   // local offset from the entity's origin
    bool is_trigger = false;  // trigger or physical collider
    bool is_enabled = true;
    LayerMask layer = 1;
    LayerMask collides_with = 0xFFFFFFFF;
};