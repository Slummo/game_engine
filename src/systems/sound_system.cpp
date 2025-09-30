#include "systems/sound_system.h"
#include "components/sound_source.h"
#include "components/sound_listener.h"
#include "components/transform.h"
#include "components/rigidbody.h"
#include "assets/sound_asset.h"
#include "contexts/event_context.h"
#include "core/engine.h"
#include "managers/context_manager.h"
#include "managers/entity_manager.h"
#include "managers/asset_manager.h"

// Utility function to set the current sound and play it
bool play_sound(AssetManager& am, SoundSource& ss, const std::string& name) {
    if (!ss.has_sound(name)) {
        return false;
    }

    if (ss.is_sound_current(name)) {
        ss.play();
        return true;
    }

    AssetID sound_id = ss.get_sound_id(name);
    SoundAsset& sound = am.get<SoundAsset>(sound_id);
    ss.set_current_sound(name, sound.buffer_id());

    return true;
}

void SoundSystem::init(Engine& engine) {
    auto& ec = engine.cm().get<EventContext>();

    EntityManager& em = engine.em();
    AssetManager& am = engine.am();

    ec.subscribe<CollisionEvent>([&](const CollisionEvent& e) {
        if (em.has_component<SoundSource>(e.a)) {
            auto& ss = em.get_component<SoundSource>(e.a);
            play_sound(am, ss, "Collision");
        }

        if (em.has_component<SoundSource>(e.b)) {
            auto& ss = em.get_component<SoundSource>(e.b);
            play_sound(am, ss, "Collision");
        }
    });

    ec.subscribe<JumpEvent>([&](const JumpEvent& e) {
        if (em.has_component<SoundSource>(e.entity)) {
            auto& ss = em.get_component<SoundSource>(e.entity);
            play_sound(am, ss, "Jump");
        }
    });
}

void SoundSystem::update(Engine& engine) {
    EntityManager& em = engine.em();

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