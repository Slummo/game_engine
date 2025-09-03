#pragma once

#include "components/icomponent.h"

struct FPController : public IComponent {
    float move_speed = 5.0f;
    float look_speed = 0.1f;
    float jump_speed = 5.0f;
    bool should_jump = false;
    bool is_grounded = true;
};
