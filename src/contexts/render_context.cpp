#include "contexts/render_context.h"
#include "components/transform.h"
#include "components/light.h"
#include "managers/asset_manager.h"

DebugObject::~DebugObject() {
    if (ebo) {
        glDeleteBuffers(1, &ebo);
    }
    if (vbo) {
        glDeleteBuffers(1, &vbo);
    }
    if (vao) {
        glDeleteVertexArrays(1, &vao);
    }
}

RenderContext::RenderContext(EntityManager& em) {
    AssetManager& am = AssetManager::instance();
    colored_line_shader_id = am.load_asset<ShaderAsset>("colored_line/");

    create_hitbox();
    create_arrow(em);
}

void RenderContext::create_hitbox() {
    // 8 verts of unit cube centered at origin
    float verts[] = {
        -0.5f, -0.5f, -0.5f,  // 0
        0.5f,  -0.5f, -0.5f,  // 1
        0.5f,  0.5f,  -0.5f,  // 2
        -0.5f, 0.5f,  -0.5f,  // 3
        -0.5f, -0.5f, 0.5f,   // 4
        0.5f,  -0.5f, 0.5f,   // 5
        0.5f,  0.5f,  0.5f,   // 6
        -0.5f, 0.5f,  0.5f    // 7
    };

    uint32_t inds[] = {
        0, 1, 1, 2, 2, 3, 3, 0,  // back rectangle
        4, 5, 5, 6, 6, 7, 7, 4,  // front rectangle
        0, 4, 1, 5, 2, 6, 3, 7   // connections
    };

    glGenVertexArrays(1, &hitbox.vao);
    glGenBuffers(1, &hitbox.vbo);
    glGenBuffers(1, &hitbox.ebo);

    glBindVertexArray(hitbox.vao);
    glBindBuffer(GL_ARRAY_BUFFER, hitbox.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(verts), verts, GL_STATIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, hitbox.ebo);
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(inds), inds, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);  // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);

    glBindVertexArray(0);
}

void RenderContext::create_arrow(EntityManager& em) {
    Transform tr;
    Light light;
    for (auto [_e, l_tr, l] : em.entities_with<Transform, Light>()) {
        if (l.type == LightType::Directional) {
            tr = l_tr;
            light = l;
            break;
        }
    }

    glm::vec3 start = tr.position();
    glm::vec3 dir = light.direction;
    float length = 7.0f;
    glm::vec3 end = start + dir * length;

    float vertices[] = {start.x, start.y, start.z, end.x, end.y, end.z};

    glGenVertexArrays(1, &arrow.vao);
    glGenBuffers(1, &arrow.vbo);

    glBindVertexArray(arrow.vao);
    glBindBuffer(GL_ARRAY_BUFFER, arrow.vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

    glEnableVertexAttribArray(0);  // position
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(float) * 3, (void*)0);

    glBindVertexArray(0);
}