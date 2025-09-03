#pragma once

#include "systems/isystem.h"

class CollisionDetectionSystem : public ISystem<CollisionContext> {
public:
    void init(EntityManager& em, CollisionContext& cc) override;
    void update(EntityManager& em, CollisionContext& cc) override;
};
