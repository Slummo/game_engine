#pragma once

#include "systems/isystem.h"

class EntityManager;
class Contact;

class CollisionResolutionSystem : public ISystem {
public:
    void update(Engine& engine) override;

private:
    void resolve_phys_contact(EntityManager& em, const Contact& c);
    void positional_correction(EntityManager& em, const Contact& c);
};
