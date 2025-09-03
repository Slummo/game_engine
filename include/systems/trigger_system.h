#pragma once

#include "systems/isystem.h"

class TriggerSystem : public ISystem<CollisionContext> {
public:
    void update(EntityManager& em, CollisionContext& cc) override;

private:
    void resolve_trigger_contact(EntityManager& em, const Contact& c);
};