#pragma once

#include "events/ievent.h"
#include "core/types/id.h"

struct JumpEvent : public IEvent {
    JumpEvent(EntityID e) : entity(e) {
    }

    EntityID entity;
};