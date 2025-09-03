#include "systems/sound_system.h"

#include <AL/al.h>

void SoundSystem::init(EntityManager& em) {
    for (auto [_e, ss, cs] : em.entities_with<SoundSource, CollisionSound>()) {
        AssetID sound_id = AssetManager::instance().load_asset<SoundAsset>(cs.sound_filename);
        ss.add_sound("Collision", sound_id);
    }
}

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

        if (ss.should_play) {
            ss.should_play = false;
            alSourcePlay(ss.source_id);
        }

        if (ss.should_pause) {
            ss.should_pause = false;
            alSourcePause(ss.source_id);
        }

        if (ss.should_stop) {
            ss.should_stop = false;
            alSourceStop(ss.source_id);
        }
    }
}