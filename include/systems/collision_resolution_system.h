#pragma once

#include "systems/isystem.h"

class CollisionResolutionSystem : public ISystem<CollisionContext, EventContext> {
public:
    void update(EntityManager& em, CollisionContext& cc, EventContext& ec) override;

private:
    void resolve_phys_contact(EntityManager& em, const Contact& c);
    void positional_correction(EntityManager& em, const Contact& c);
};
