#pragma once

#include "systems/isystem.h"
#include "contexts/camera_context.h"

class CameraSystem : public ISystem<CameraContext> {
public:
    void update(EntityManager& em, CameraContext& cc) override;
};