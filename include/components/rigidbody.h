#pragma once

#include "components/icomponent.h"

#include <glm/glm.hpp>

struct RigidBody : public IComponent {
    RigidBody(float m = 1.0f, bool _is_static = false, bool _is_kinematic = false)
        : mass(m), inv_mass(0.0f), is_kinematic(_is_kinematic), is_static(_is_static) {
        set_mass(_is_static ? 0.0f : m);
    }

    float mass = 1.0f;          // Mass > 0 for dynamic bodies
    float inv_mass = 1.0f;      // 1.0f / mass
    bool is_kinematic = false;  // Moved by user, not by physics
    bool is_static = false;     // Immovable

    glm::vec3 velocity{0.0f};
    glm::vec3 force_accum{0.0f};  // accumulated forces for this frame

    // Material
    float restitution = 0.0f;  // bounciness [0..1]
    float friction = 0.5f;

    // Damping
    float linear_damping = 0.2f;  // simple damping applied each frame

    void set_mass(float m) {
        mass = m;
        inv_mass = (m > 0.0f && !is_static && !is_kinematic) ? (1.0f / m) : 0.0f;
    }

    // Semi-implicit Euler integration
    // v += (force * inv_mass) * dt
    void apply_force(const glm::vec3& force, float dt) {
        force_accum += force;

        if (inv_mass > 0.0f) {
            glm::vec3 acceleration = force_accum * inv_mass;
            velocity += acceleration * dt;
        }
    }

    // v += impulse * inv_mass
    void apply_impulse(const glm::vec3& impulse) {
        if (inv_mass > 0.0f) {
            velocity += impulse * inv_mass;
        }
    }

    // Exponential damping
    void apply_damping(float dt) {
        float damping_factor = std::exp(-linear_damping * dt);
        velocity *= damping_factor;
    }

    void clear_forces() {
        force_accum = glm::vec3(0.0f);
    }
};