#include "systems/sound_system.h"

void SoundSystem::update(EntityManager& em) {
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