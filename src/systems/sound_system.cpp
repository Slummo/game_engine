#include "systems/sound_system.h"

void SoundSystem::update(ECS& ecs, float /*dt*/) {
    for (auto [_e, tr, rb, sl] : ecs.entities_with<TransformComponent, RigidBodyComponent, SoundListenerComponent>()) {
        sl.set_owner_position(tr.position());
        sl.set_owner_velocity(rb.velocity);
    }

    for (auto [e, tr, ss] : ecs.entities_with<TransformComponent, SoundSourceComponent>()) {
        ss.set_owner_position(tr.position());

        if (ecs.has_component<RigidBodyComponent>(e)) {
            auto& rb = ecs.get_component<RigidBodyComponent>(e);
            ss.set_owner_velocity(rb.velocity);
        }
    }
}