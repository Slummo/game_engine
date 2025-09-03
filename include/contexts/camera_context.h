#pragma once

#include "contexts/icontext.h"
#include "components/camera.h"

struct CameraContext : public IContext {
    Camera& main_camera;

    CameraContext(Camera& main_camera) : main_camera(main_camera) {
    }
};