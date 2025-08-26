#pragma once

#include "systems/isystem.h"

#include <glm/glm.hpp>

class CameraSystem : public ISystem {
public:
    CameraSystem() = delete;
    CameraSystem(CameraComponent& cam);

    void update(ECS& ecs, float dt) override;

    void set_main_camera(CameraComponent& cam);

private:
    CameraComponent& m_main_camera;
};