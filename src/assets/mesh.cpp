#include "assets/mesh.h"
#include "managers/asset_manager.h"
#include "core/log.h"

#include <glad/glad.h>
#include <iostream>

static void print_gl_error_if_any(const char* context) {
    GLenum err = glGetError();
    if (err != GL_NO_ERROR) {
        ERR("[Mesh] Gl error after " << context << " : 0x" << std::hex << err << std::dec)
    }
}

size_t vertex_stride(VertexFormat format) {
    switch (format) {
        case VertexFormat::POS_TEX: {
            return sizeof(Vertex_PT);
        }
        case VertexFormat::POS_NOR_TEX: {
            return sizeof(Vertex_PNT);
        }
        default: {
            throw std::runtime_error("Unknown vertex format");
        }
    }
}

Mesh::Mesh(std::string name, VertexFormat format, const void* vertices, size_t vertices_count, const void* indices,
           size_t indices_count, AssetID material_id)
    : m_name(name.empty() ? "unnamed_mesh" : std::move(name)),
      m_format(format),
      m_vertices_num(vertices_count),
      m_indices_num(indices_count),
      m_material_id(material_id) {
    compute_aabb(vertices);
    upload_to_GPU(vertices, indices);
}

std::shared_ptr<Mesh> Mesh::create_fallback() {
    return std::make_shared<Mesh>("fallback_mesh", VertexFormat::POS_TEX, nullptr, 0, nullptr, 0,
                                  AssetManager::instance().get_fallback_id<Material>());
}

AssetID Mesh::create_cube_PNT(AssetID material_id, float uv_scale_x, float uv_scale_y) {
    // 24 m_vertices (4 verts per face), each vertex: pos3, nor3, tex2
    std::vector<Vertex_PNT> vertices = {
        // Back face (-Z)
        {{-0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f * uv_scale_x, 0.0f * uv_scale_y}},
        {{0.5f, -0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f * uv_scale_x, 0.0f * uv_scale_y}},
        {{0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {1.0f * uv_scale_x, 1.0f * uv_scale_y}},
        {{-0.5f, 0.5f, -0.5f}, {0.0f, 0.0f, -1.0f}, {0.0f * uv_scale_x, 1.0f * uv_scale_y}},

        // Front face (+Z)
        {{-0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f * uv_scale_x, 0.0f * uv_scale_y}},
        {{0.5f, -0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f * uv_scale_x, 0.0f * uv_scale_y}},
        {{0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {1.0f * uv_scale_x, 1.0f * uv_scale_y}},
        {{-0.5f, 0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f * uv_scale_x, 1.0f * uv_scale_y}},

        // Left face (-X)
        {{-0.5f, 0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f * uv_scale_x, 0.0f * uv_scale_y}},
        {{-0.5f, 0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {1.0f * uv_scale_x, 1.0f * uv_scale_y}},
        {{-0.5f, -0.5f, -0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f * uv_scale_x, 1.0f * uv_scale_y}},
        {{-0.5f, -0.5f, 0.5f}, {-1.0f, 0.0f, 0.0f}, {0.0f * uv_scale_x, 0.0f * uv_scale_y}},

        // Right face (+X)
        {{0.5f, 0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f * uv_scale_x, 0.0f * uv_scale_y}},
        {{0.5f, 0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f * uv_scale_x, 1.0f * uv_scale_y}},
        {{0.5f, -0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f * uv_scale_x, 1.0f * uv_scale_y}},
        {{0.5f, -0.5f, 0.5f}, {1.0f, 0.0f, 0.0f}, {0.0f * uv_scale_x, 0.0f * uv_scale_y}},

        // Bottom face (-Y)
        {{-0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f * uv_scale_x, 1.0f * uv_scale_y}},
        {{0.5f, -0.5f, -0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f * uv_scale_x, 1.0f * uv_scale_y}},
        {{0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {1.0f * uv_scale_x, 0.0f * uv_scale_y}},
        {{-0.5f, -0.5f, 0.5f}, {0.0f, -1.0f, 0.0f}, {0.0f * uv_scale_x, 0.0f * uv_scale_y}},

        // Top face (+Y)
        {{-0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f * uv_scale_x, 0.0f * uv_scale_y}},
        {{0.5f, 0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f * uv_scale_x, 0.0f * uv_scale_y}},
        {{0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {1.0f * uv_scale_x, 1.0f * uv_scale_y}},
        {{-0.5f, 0.5f, 0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f * uv_scale_x, 1.0f * uv_scale_y}},
    };

    std::vector<uint32_t> indices = {
        0,  1,  2,  2,  3,  0,   // back
        4,  5,  6,  6,  7,  4,   // front
        8,  9,  10, 10, 11, 8,   // left
        12, 13, 14, 14, 15, 12,  // right
        16, 17, 18, 18, 19, 16,  // bottom
        20, 21, 22, 22, 23, 20   // top
    };

    return AssetManager::instance().add_asset<Mesh>("cube", VertexFormat::POS_NOR_TEX, vertices.data(), vertices.size(),
                                                    indices.data(), indices.size(), material_id);
}

AssetID Mesh::create_cube_PNT(float uv_scale_x, float uv_scale_y) {
    return Mesh::create_cube_PNT(AssetManager::instance().get_fallback_id<Material>(), uv_scale_x, uv_scale_y);
}

AssetID Mesh::create_cube_PT(AssetID material_id, float uv_scale_x, float uv_scale_y) {
    // 24 m_vertices (4 verts per face), each vertex: pos3, tex2
    std::vector<Vertex_PT> vertices = {
        // Back face (-Z)
        {{-0.5f, -0.5f, -0.5f}, {0.0f * uv_scale_x, 0.0f * uv_scale_y}},
        {{0.5f, -0.5f, -0.5f}, {1.0f * uv_scale_x, 0.0f * uv_scale_y}},
        {{0.5f, 0.5f, -0.5f}, {1.0f * uv_scale_x, 1.0f * uv_scale_y}},
        {{-0.5f, 0.5f, -0.5f}, {0.0f * uv_scale_x, 1.0f * uv_scale_y}},

        // Front face (+Z)
        {{-0.5f, -0.5f, 0.5f}, {0.0f * uv_scale_x, 0.0f * uv_scale_y}},
        {{0.5f, -0.5f, 0.5f}, {1.0f * uv_scale_x, 0.0f * uv_scale_y}},
        {{0.5f, 0.5f, 0.5f}, {1.0f * uv_scale_x, 1.0f * uv_scale_y}},
        {{-0.5f, 0.5f, 0.5f}, {0.0f * uv_scale_x, 1.0f * uv_scale_y}},

        // Left face (-X)
        {{-0.5f, 0.5f, 0.5f}, {1.0f * uv_scale_x, 0.0f * uv_scale_y}},
        {{-0.5f, 0.5f, -0.5f}, {1.0f * uv_scale_x, 1.0f * uv_scale_y}},
        {{-0.5f, -0.5f, -0.5f}, {0.0f * uv_scale_x, 1.0f * uv_scale_y}},
        {{-0.5f, -0.5f, 0.5f}, {0.0f * uv_scale_x, 0.0f * uv_scale_y}},

        // Right face (+X)
        {{0.5f, 0.5f, 0.5f}, {1.0f * uv_scale_x, 0.0f * uv_scale_y}},
        {{0.5f, 0.5f, -0.5f}, {1.0f * uv_scale_x, 1.0f * uv_scale_y}},
        {{0.5f, -0.5f, -0.5f}, {0.0f * uv_scale_x, 1.0f * uv_scale_y}},
        {{0.5f, -0.5f, 0.5f}, {0.0f * uv_scale_x, 0.0f * uv_scale_y}},

        // Bottom face (-Y)
        {{-0.5f, -0.5f, -0.5f}, {0.0f * uv_scale_x, 1.0f * uv_scale_y}},
        {{0.5f, -0.5f, -0.5f}, {1.0f * uv_scale_x, 1.0f * uv_scale_y}},
        {{0.5f, -0.5f, 0.5f}, {1.0f * uv_scale_x, 0.0f * uv_scale_y}},
        {{-0.5f, -0.5f, 0.5f}, {0.0f * uv_scale_x, 0.0f * uv_scale_y}},

        // Top face (+Y)
        {{-0.5f, 0.5f, -0.5f}, {0.0f * uv_scale_x, 0.0f * uv_scale_y}},
        {{0.5f, 0.5f, -0.5f}, {1.0f * uv_scale_x, 0.0f * uv_scale_y}},
        {{0.5f, 0.5f, 0.5f}, {1.0f * uv_scale_x, 1.0f * uv_scale_y}},
        {{-0.5f, 0.5f, 0.5f}, {0.0f * uv_scale_x, 1.0f * uv_scale_y}},
    };

    std::vector<uint32_t> indices = {
        0,  1,  2,  2,  3,  0,   // back
        4,  5,  6,  6,  7,  4,   // front
        8,  9,  10, 10, 11, 8,   // left
        12, 13, 14, 14, 15, 12,  // right
        16, 17, 18, 18, 19, 16,  // bottom
        20, 21, 22, 22, 23, 20   // top
    };

    return AssetManager::instance().add_asset<Mesh>("cube", VertexFormat::POS_TEX, vertices.data(), vertices.size(),
                                                    indices.data(), indices.size(), material_id);
}

AssetID Mesh::create_cube_PT(float uv_scale_x, float uv_scale_y) {
    return Mesh::create_cube_PT(AssetManager::instance().get_fallback_id<Material>(), uv_scale_x, uv_scale_y);
}

void Mesh::draw() const {
    if (m_vao == 0) {
        return;
    }

    glBindVertexArray(m_vao);
    glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(m_indices_num), GL_UNSIGNED_INT, nullptr);
    glBindVertexArray(0);
}

void Mesh::destroy() {
    if (m_ebo) {
        glDeleteBuffers(1, &m_ebo);
        m_ebo = 0;
    }
    if (m_vbo) {
        glDeleteBuffers(1, &m_vbo);
        m_vbo = 0;
    }
    if (m_vao) {
        glDeleteVertexArrays(1, &m_vao);
        m_vao = 0;
    }
}

Mesh::~Mesh() {
    destroy();
}

int32_t Mesh::index_count() const {
    return static_cast<int32_t>(m_indices_num);
}

AssetID Mesh::material_id() const {
    return m_material_id;
}

const AABB& Mesh::aabb() const {
    return m_aabb;
}

std::ostream& Mesh::print(std::ostream& os) const {
    return os << "Mesh(name: " << m_name << ", vertices_num: " << m_vertices_num << ", indices_num: " << m_indices_num
              << ", material(asset_id = " << m_material_id
              << "): " << AssetManager::instance().asset_to_string(m_material_id) << ")";
}

void Mesh::compute_aabb(const void* vertices) {
    if (m_vertices_num == 0 || !vertices) {
        m_aabb = {glm::vec3(0.0f), glm::vec3(0.0f)};
        return;
    }

    // Interpret vertices based on the format
    glm::vec3 min(std::numeric_limits<float>::max());
    glm::vec3 max(-std::numeric_limits<float>::max());

    switch (m_format) {
        case VertexFormat::POS_TEX: {
            auto vtx = static_cast<const Vertex_PT*>(vertices);
            for (size_t i = 0; i < m_vertices_num; i++) {
                min = glm::min(min, vtx[i].pos3);
                max = glm::max(max, vtx[i].pos3);
            }
            break;
        }
        case VertexFormat::POS_NOR_TEX: {
            auto vtx = static_cast<const Vertex_PNT*>(vertices);
            for (size_t i = 0; i < m_vertices_num; i++) {
                min = glm::min(min, vtx[i].pos3);
                max = glm::max(max, vtx[i].pos3);
            }
            break;
        }
        default: {
            throw std::runtime_error("Unknown vertex format");
        }
    }

    m_aabb = {min, max};
}

void Mesh::upload_to_GPU(const void* vertices, const void* indices) {
    if (!vertices || !indices) {
        return;
    }

    if (m_vao || m_vbo || m_ebo) {
        destroy();
    }

    // Create buffers
    glGenVertexArrays(1, &m_vao);
    glGenBuffers(1, &m_vbo);
    if (m_indices_num != 0) {
        glGenBuffers(1, &m_ebo);
    }

    glBindVertexArray(m_vao);

    size_t stride = vertex_stride(m_format);

    // VBO
    glBindBuffer(GL_ARRAY_BUFFER, m_vbo);
    glBufferData(GL_ARRAY_BUFFER, m_vertices_num * stride, vertices, GL_STATIC_DRAW);

    // EBO
    if (m_indices_num != 0) {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, m_indices_num * sizeof(uint32_t), indices, GL_STATIC_DRAW);
    }

    // Vertex attributes
    switch (m_format) {
        case VertexFormat::POS_TEX: {
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const void*)offsetof(Vertex_PT, pos3));
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, stride, (const void*)offsetof(Vertex_PT, tex2));
            break;
        }
        case VertexFormat::POS_NOR_TEX: {
            glEnableVertexAttribArray(0);
            glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, stride, (const void*)offsetof(Vertex_PNT, pos3));
            glEnableVertexAttribArray(1);
            glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, stride, (const void*)offsetof(Vertex_PNT, nor3));
            glEnableVertexAttribArray(2);
            glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, stride, (const void*)offsetof(Vertex_PNT, tex2));
            break;
        }
        default: {
            throw std::runtime_error("Unknown vertex format");
        }
    }

    glBindVertexArray(0);
    glBindBuffer(GL_ARRAY_BUFFER, 0);

    print_gl_error_if_any("Mesh::upload_to_GPU");
}