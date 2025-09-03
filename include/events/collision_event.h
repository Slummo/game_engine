#pragma once

#include "events/ievent.h"
#include "core/types/id.h"

struct CollisionEvent : public IEvent {
    CollisionEvent(EntityID a, EntityID b) : a(a), b(b) {
    }

    EntityID a;
    EntityID b;
};