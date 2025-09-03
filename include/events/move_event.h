#pragma once

#include "events/ievent.h"
#include "core/types/id.h"

#include <glm/glm.hpp>

struct MoveEvent : public IEvent {
    MoveEvent(EntityID e, glm::vec3 direction) : entity(e), direction(std::move(direction)) {
    }

    EntityID entity;
    glm::vec3 direction;
};