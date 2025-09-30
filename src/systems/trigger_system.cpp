#include "systems/trigger_system.h"
#include "contexts/collision_context.h"
#include "core/types/contact.h"
#include "core/engine.h"
#include "managers/context_manager.h"
#include "managers/entity_manager.h"

void TriggerSystem::update(Engine& engine) {
    auto& cc = engine.cm().get<CollisionContext>();

    EntityManager& em = engine.em();

    for (const Contact& c : cc.contacts) {
        if (c.is_trigger) {
            resolve_trigger_contact(em, c);
        }
    }
}

void TriggerSystem::resolve_trigger_contact(EntityManager& /*em*/, const Contact& /*c*/) {
    // TODO
}