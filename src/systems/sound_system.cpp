#include "systems/sound_system.h"

void SoundSystem::init(EntityManager& em, EventContext& ec) {
    ec.subscribe<CollisionEvent>([&](const CollisionEvent& e) {
        if (em.has_component<SoundSource>(e.a)) {
            auto& ss = em.get_component<SoundSource>(e.a);
            ss.play_sound("Collision");
        }

        if (em.has_component<SoundSource>(e.b)) {
            auto& ss = em.get_component<SoundSource>(e.b);
            ss.play_sound("Collision");
        }
    });

    ec.subscribe<JumpEvent>([&](const JumpEvent& e) {
        if (em.has_component<SoundSource>(e.entity)) {
            auto& ss = em.get_component<SoundSource>(e.entity);
            ss.play_sound("Jump");
        }
    });
}

void SoundSystem::update(EntityManager& em, EventContext& /*ec*/) {
    for (auto [_e, tr, rb, sl] : em.entities_with<Transform, RigidBody, SoundListener>()) {
        sl.set_owner_position(tr.position());
        sl.set_owner_velocity(rb.velocity);
    }

    for (auto [e, tr, ss] : em.entities_with<Transform, SoundSource>()) {
        ss.set_owner_position(tr.position());
        if (em.has_component<RigidBody>(e)) {
            auto& rb = em.get_component<RigidBody>(e);
            ss.set_owner_velocity(rb.velocity);
        }
    }
}