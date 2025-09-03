#include "systems/light_system.h"

void LightSystem::init(EntityManager& em) {
    for (auto [_e, tr, l] : em.entities_with<Transform, Light>()) {
        if (l.type == LightType::Directional) {
            l.direction = tr.rotation() * glm::vec3(0.0f, 0.0f, -1.0f);  // forward vec (-Z);
        }
    }
}