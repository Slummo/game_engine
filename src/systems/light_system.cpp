#include "systems/light_system.h"
#include "components/transform.h"
#include "components/light.h"
#include "core/engine.h"
#include "managers/entity_manager.h"

void LightSystem::init(Engine& engine) {
    EntityManager& em = engine.em();

    for (auto [_e, tr, l] : em.entities_with<Transform, Light>()) {
        if (l.type == LightType::Directional) {
            l.direction = tr.rotation() * glm::vec3(0.0f, 0.0f, -1.0f);  // forward vec (-Z);
        }
    }
}