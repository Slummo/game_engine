#include "systems/rotation_system.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

void RotationSystem::update(ECS &ecs, float dt) {
    for (auto [_e, tr, rot] : ecs.entities_with<TransformComponent, RotatorComponent>()) {
        // Compute incremental rotation quaternion
        glm::quat delta = glm::angleAxis(glm::radians(rot.speed_deg * dt), glm::normalize(rot.axis));

        tr.update_rotation(delta);
    }
}