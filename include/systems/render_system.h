#pragma once

#include "systems/isystem.h"

#include <glm/glm.hpp>
#include <unordered_map>
#include <cstdint>

class RenderSystem : public ISystem<RenderContext, InputContext> {
public:
    void init(EntityManager& em, RenderContext& rc, InputContext& ic) override;
    void update(EntityManager& em, RenderContext& rc, InputContext& ic) override;
    void shutdown(EntityManager& em, RenderContext& rc, InputContext& ic) override;

private:
    void render_scene(EntityManager& em, Camera& cam_c, RenderContext& rc);
    void render_hitboxes(EntityManager& em, Camera& cam_c, RenderContext& rc);
    void render_debug(EntityManager& em, Camera& cam_c, RenderContext& rc);

    void create_wire_cube(RenderContext& rc);
    void destroy_wire_cube(RenderContext& rc);
    void create_arrow(EntityManager& em, RenderContext& rc);
    void destroy_arrow(RenderContext& rc);
};
