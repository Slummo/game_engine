#include "systems/trigger_system.h"

void TriggerSystem::update(EntityManager& em, CollisionContext& cc) {
    for (const Contact& c : cc.contacts) {
        if (c.is_trigger) {
            resolve_trigger_contact(em, c);
        }
    }
}

void TriggerSystem::resolve_trigger_contact(EntityManager& /*em*/, const Contact& /*c*/) {
    // TODO
}