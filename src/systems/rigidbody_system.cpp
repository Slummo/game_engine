#include "systems/rigidbody_system.h"

void RigidBodySystem::update(EntityManager& em, PhysicsContext& pc) {
    for (auto [e, tr, rb] : em.entities_with<Transform, RigidBody>()) {
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
        bool is_grounded_player = em.has_component<FPController>(e) && em.get_component<FPController>(e).is_grounded;
        if (!is_grounded_player) {
            rb.apply_force(pc.gravity * rb.mass, pc.dt);
        }

        rb.apply_damping(pc.dt);

        // Update position
        tr.update_position(rb.velocity * pc.dt);

        // Clear per-frame forces
        rb.clear_forces();
    }
}