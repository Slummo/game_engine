#include "systems/light_system.h"

void LightSystem::init(ECS& ecs) {
    for (auto [_e, tr, l] : ecs.entities_with<TransformComponent, LightComponent>()) {
        if (l.type == LightType::Directional) {
            l.direction = tr.rotation() * glm::vec3(0.0f, 0.0f, -1.0f);  // forward vec (-Z);
        }
    }
}