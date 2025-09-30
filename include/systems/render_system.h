#pragma once

#include "systems/isystem.h"

class EntityManager;
class AssetManager;
class Camera;
class DebugContext;
class InputContext;

#include <glm/glm.hpp>
#include <unordered_map>
#include <cstdint>

class RenderSystem : public ISystem {
public:
    void init(Engine& engine) override;
    void update(Engine& engine) override;

private:
    void render_scene(EntityManager& em, AssetManager& am, Camera& cam_c, DebugContext& rc);
    void render_debug(EntityManager& em, AssetManager& am, Camera& cam_c, InputContext& ic, DebugContext& rc);
    void render_gui();
};
