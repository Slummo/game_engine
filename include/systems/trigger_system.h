#pragma once

#include "systems/isystem.h"

class EntityManager;
class Contact;

class TriggerSystem : public ISystem {
public:
    void update(Engine& engine) override;

private:
    void resolve_trigger_contact(EntityManager& em, const Contact& c);
};