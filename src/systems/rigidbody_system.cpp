#include "systems/rigidbody_system.h"
#include "components/transform.h"
#include "components/rigidbody.h"
#include "components/fp_controller.h"
#include "contexts/physics_context.h"
#include "contexts/event_context.h"
#include "core/engine.h"
#include "managers/context_manager.h"
#include "managers/entity_manager.h"

#define RB_EPS 1e-6f
#define JMSRF 0.65f                // Jumping movement speed reduction factor
#define BRAKE_ACCEL 20.0f          // m/s^2 large to stop quickly
#define STOP_SPEED_THRESHOLD 0.2f  // below this, snap to zero

void RigidBodySystem::init(Engine& engine) {
    auto& pc = engine.cm().get<PhysicsContext>();
    auto& ec = engine.cm().get<EventContext>();

    EntityManager& em = engine.em();

    ec.subscribe<MoveEvent>([&](const MoveEvent& e) {
        auto [rb, fpc] = em.get_components<RigidBody, FPController>(e.entity);

        // Current horizontal velocity and speed
        glm::vec3 horiz_vel = rb.velocity;
        horiz_vel.y = 0.0f;
        float speed = glm::length(horiz_vel);

        const glm::vec3& move_dir = e.direction;
        bool is_moving = glm::dot(move_dir, move_dir) > RB_EPS;
        glm::vec3 dv;
        if (is_moving) {
            glm::vec3 desired_vel_h = move_dir * fpc.move_speed;
            if (!fpc.is_grounded) {
                desired_vel_h *= JMSRF;
            }
            dv = desired_vel_h - horiz_vel;
        } else {
            float max_dv = BRAKE_ACCEL * pc.dt;

            if (speed <= STOP_SPEED_THRESHOLD) {
                rb.apply_impulse(-horiz_vel * rb.mass);
            } else {
                // reduce speed by up to max_dv
                dv = -horiz_vel * (max_dv / speed);
            }
        }

        rb.apply_impulse(dv * rb.mass);
    });

    ec.subscribe<JumpEvent>([&](const JumpEvent& e) {
        auto [rb, fpc] = em.get_components<RigidBody, FPController>(e.entity);
        rb.apply_impulse(glm::vec3(0.0f, fpc.jump_speed * rb.mass, 0.0f));
        fpc.is_grounded = false;
    });
}

void RigidBodySystem::update(Engine& engine) {
    auto& pc = engine.cm().get<PhysicsContext>();

    EntityManager& em = engine.em();

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
        bool is_grounded = em.has_component<FPController>(e) && em.get_component<FPController>(e).is_grounded;
        if (!is_grounded) {
            rb.apply_force(pc.gravity * rb.mass, pc.dt);
        }

        rb.apply_damping(pc.dt);
        tr.update_position(rb.velocity * pc.dt);
        rb.clear_forces();
    }
}