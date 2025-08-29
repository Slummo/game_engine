#pragma once

#include "components/icomponent.h"

#include <AL/al.h>
#include <glm/glm.hpp>

struct SoundListenerComponent : public IComponent {
    void set_owner_position(const glm::vec3& owner_position) {
        alListener3f(AL_POSITION, owner_position.x, owner_position.y, owner_position.z);
    }

    void set_owner_velocity(const glm::vec3& owner_velocity) {
        alListener3f(AL_VELOCITY, owner_velocity.x, owner_velocity.y, owner_velocity.z);
    }
};