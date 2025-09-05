#pragma once

#include "systems/isystem.h"

#include <glm/glm.hpp>
#include <unordered_map>
#include <cstdint>

class RenderSystem : public ISystem<RenderContext, CameraContext, InputContext> {
public:
    void init(EntityManager& em, RenderContext& rc, CameraContext& cc, InputContext& ic) override;
    void update(EntityManager& em, RenderContext& rc, CameraContext& cc, InputContext& ic) override;

private:
    void render_scene(EntityManager& em, Camera& cam_c, RenderContext& rc);
    void render_hitboxes(EntityManager& em, Camera& cam_c, RenderContext& rc);
    void render_debug(EntityManager& em, Camera& cam_c, RenderContext& rc);
    void render_text(RenderContext& rc, const std::string& text, float x, float y, float scale, const glm::vec3& color);
};
