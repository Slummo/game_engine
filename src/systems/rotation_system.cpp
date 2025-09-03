#include "systems/rotation_system.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

void RotationSystem::update(EntityManager& em, PhysicsContext& pc) {
    for (auto [_e, tr, rot] : em.entities_with<Transform, Rotator>()) {
        // Compute incremental rotation quaternion
        glm::quat delta = glm::angleAxis(glm::radians(rot.speed_deg * pc.dt), glm::normalize(rot.axis));

        tr.update_rotation(delta);
    }
}