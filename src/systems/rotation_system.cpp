#include "systems/rotation_system.h"
#include "components/transform.h"
#include "components/rotator.h"
#include "contexts/physics_context.h"
#include "core/engine.h"
#include "managers/context_manager.h"
#include "managers/entity_manager.h"

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>

void RotationSystem::update(Engine& engine) {
    auto& pc = engine.cm().get<PhysicsContext>();

    EntityManager& em = engine.em();

    for (auto [_e, tr, rot] : em.entities_with<Transform, Rotator>()) {
        // Compute incremental rotation quaternion
        glm::quat delta = glm::angleAxis(glm::radians(rot.speed_deg * pc.dt), glm::normalize(rot.axis));

        tr.update_rotation(delta);
    }
}