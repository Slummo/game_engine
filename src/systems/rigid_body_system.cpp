#include "systems/rigid_body_system.h"

#include <algorithm>
#include <iostream>

RigidBodySystem::RigidBodySystem(const glm::vec3& gravity) : m_gravity(gravity) {
}

void RigidBodySystem::update(ECS& /*ecs*/, float /*dt*/) {
}

void RigidBodySystem::fixed_update(ECS& ecs, float fixed_dt) {
    if (fixed_dt <= 0.0f) {
        return;
    }

    for (auto [e, tr, rb] : ecs.entities_with<TransformComponent, RigidBodyComponent>()) {
        // Skip static bodies
        if (rb.is_static) {
            rb.velocity = glm::vec3(0.0f);
            rb.clear_forces();
            continue;
        }

        // Skip kinematic as well
        if (rb.is_kinematic) {
            rb.clear_forces();
            continue;
        }

        // Apply gravity (not to a grounded player)
        bool is_grounded_player = ecs.has_component<PlayerComponent>(e) &&
                                  ecs.has_component<FPControllerComponent>(e) &&
                                  ecs.get_component<FPControllerComponent>(e).is_grounded;
        if (!is_grounded_player) {
            rb.apply_force(m_gravity * rb.mass, fixed_dt);
        }

        rb.apply_damping(fixed_dt);

        // Update position
        tr.update_position(rb.velocity * fixed_dt);

        // Clear per-frame forces
        rb.clear_forces();
    }
}