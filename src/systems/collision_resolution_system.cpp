#include "systems/collision_resolution_system.h"
#include "components/fp_controller.h"
#include "components/transform.h"
#include "components/rigidbody.h"
#include "components/collider.h"
#include "components/player.h"
#include "contexts/collision_context.h"
#include "contexts/event_context.h"
#include "core/engine.h"
#include "managers/context_manager.h"
#include "managers/entity_manager.h"

#define GROUND_NORMAL_THRESHOLD 0.75f
#define COR_PER 0.1f  // positional correction percentage
#define SLOP 0.01f    // penetration allowance

void CollisionResolutionSystem::update(Engine& engine) {
    auto& cc = engine.cm().get<CollisionContext>();
    auto& ec = engine.cm().get<EventContext>();

    EntityManager& em = engine.em();

    for (auto [_e, fpc] : em.entities_with<FPController>()) {
        fpc.is_grounded = false;
    }

    for (const Contact& c : cc.contacts) {
        if (!c.is_trigger) {
            resolve_phys_contact(em, c);
            positional_correction(em, c);

            // Play collisions sounds
            auto& col_a = em.get_component<Collider>(c.a);
            auto& col_b = em.get_component<Collider>(c.b);
            if (col_a.layer != Layers::Ground && col_b.layer != Layers::Ground) {
                ec.emit(CollisionEvent{c.a, c.b});
            }
        }
    }

    ec.dispatch();
}

void CollisionResolutionSystem::resolve_phys_contact(EntityManager& em, const Contact& c) {
    // Get components
    if (!em.has_component<RigidBody>(c.a) || !em.has_component<RigidBody>(c.b)) {
        return;
    }

    auto& a_rb = em.get_component<RigidBody>(c.a);
    auto& b_rb = em.get_component<RigidBody>(c.b);

    // Skip static/static
    if (a_rb.is_static && b_rb.is_static) {
        return;
    }

    // Determine if one of the two entities is the player
    bool is_a_player = em.has_component<Player>(c.a);
    bool is_b_player = em.has_component<Player>(c.b);

    float normal_y_for_player = is_a_player ? -c.normal.y : c.normal.y;  // flip if player is A
    if (is_a_player || is_b_player) {
        auto& fpc = em.get_component<FPController>(is_a_player ? c.a : c.b);

        // Only set grounded if moving downward and hitting mostly horizontal surface
        if (normal_y_for_player > GROUND_NORMAL_THRESHOLD) {
            fpc.is_grounded = true;
        }
    }

    // Relative velocity at contact
    glm::vec3 rv = b_rb.velocity - a_rb.velocity;
    float vel_along_normal = glm::dot(rv, c.normal);

    // Do not resolve separating contacts
    if (vel_along_normal > 0.0f) {
        return;
    }

    // Restitution
    float e = std::min(a_rb.restitution, b_rb.restitution);
    float inv_mass_sum = a_rb.inv_mass + b_rb.inv_mass;
    if (inv_mass_sum <= 0.0f) {
        return;
    }

    // Compute impulse
    float j = -(1.0f + e) * vel_along_normal;
    j /= inv_mass_sum;
    glm::vec3 impulse = j * c.normal;

    if (!a_rb.is_static && !a_rb.is_kinematic) {
        a_rb.apply_impulse(-impulse);
    }
    if (!b_rb.is_static && !b_rb.is_kinematic) {
        b_rb.apply_impulse(impulse);
    }

    // Simple friction with approximated Coulomb model
    glm::vec3 tangent = rv - vel_along_normal * c.normal;
    float tlen2 = glm::dot(tangent, tangent);
    if (tlen2 > 1e-8f) {
        tangent = glm::normalize(tangent);
        float mu = std::sqrt(a_rb.friction * b_rb.friction);
        float jt = -glm::dot(rv, tangent);
        jt /= inv_mass_sum;

        // Clamp friction impulse
        float jt_max = j * mu;
        jt = std::clamp(jt, -jt_max, jt_max);
        glm::vec3 friction_impulse = jt * tangent;
        if (!a_rb.is_static && !a_rb.is_kinematic) {
            a_rb.apply_impulse(-friction_impulse);
        }
        if (!b_rb.is_static && !b_rb.is_kinematic) {
            b_rb.apply_impulse(friction_impulse);
        }
    }
}

void CollisionResolutionSystem::positional_correction(EntityManager& em, const Contact& c) {
    if (!em.has_components<Transform, RigidBody>(c.a) || !em.has_components<Transform, RigidBody>(c.b)) {
        return;
    }

    auto [a_tr, a_rb] = em.get_components<Transform, RigidBody>(c.a);
    auto [b_tr, b_rb] = em.get_components<Transform, RigidBody>(c.b);

    if (a_rb.is_static && b_rb.is_static) {
        return;
    }

    float inv_mass_sum = a_rb.inv_mass + b_rb.inv_mass;
    if (inv_mass_sum == 0.0f) {
        return;
    }

    float correction_mag = std::max(c.penetration - SLOP, 0.0f) / inv_mass_sum * COR_PER;
    glm::vec3 correction = correction_mag * c.normal;

    // move A opposite to the normal
    if (!a_rb.is_static && !a_rb.is_kinematic) {
        a_tr.update_position(-correction * a_rb.inv_mass);
    }

    // move B along the normal
    if (!b_rb.is_static && !b_rb.is_kinematic) {
        b_tr.update_position(correction * b_rb.inv_mass);
    }
}
