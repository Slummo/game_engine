#pragma once

#include "assets/iasset.h"
#include "core/types/aabb.h"

#include <vector>
#include <string>
#include <memory>

enum class VertexFormat { POS_TEX, POS_NOR_TEX };

struct Vertex_PT {
    glm::vec3 pos3;
    glm::vec2 tex2;
};

struct Vertex_PNT {
    glm::vec3 pos3;
    glm::vec3 nor3;
    glm::vec2 tex2;
    // int32_t bone_ids[4] = {-1};
    // float bone_weights[4] = {0.0f};

    // void add_bone_data(int32_t bone_id, float bone_weight) {
    //     for (int32_t i = 0; i < 4; i++) {
    //         if (bone_ids[i] < 0) {
    //             bone_ids[i] = bone_id;
    //             bone_weights[i] = bone_weight;
    //             return;
    //         }
    //     }

    //     // If more than 4 bones replace the one with smallest weight
    //     int32_t smallest_id = 0;
    //     for (int32_t i = 1; i < 4; i++) {
    //         if (bone_weights[i] < bone_weights[smallest_id]) {
    //             smallest_id = i;
    //         }
    //     }

    //     // Replace only if the new weight is higher that the smallest
    //     if (bone_weights[smallest_id] < bone_weight) {
    //         bone_ids[smallest_id] = bone_id;
    //         bone_weights[smallest_id] = bone_weight;
    //     }
    // }
};

size_t vertex_stride(VertexFormat format);

struct BoneInfo {
    glm::mat4 offset_matrix;         // Converts from mesh space to bone space
    glm::mat4 final_transformation;  // Transformation product of the scene hierarchy
};

struct MeshAsset : public IAsset {
public:
    MeshAsset(std::string name, VertexFormat format, const void* vertices, size_t vertices_count, const void* indices,
              size_t indices_count, AssetID material_id);

    static std::shared_ptr<MeshAsset> create_fallback();

    static AssetID create_cube_PNT(AssetID material_id, float uv_scale_x = 1.0f, float uv_scale_y = 1.0f);
    static AssetID create_cube_PNT(float uv_scale_x = 1.0f, float uv_scale_y = 1.0f);

    static AssetID create_cube_PT(AssetID material_id, float uv_scale_x = 1.0f, float uv_scale_y = 1.0f);
    static AssetID create_cube_PT(float uv_scale_x = 1.0f, float uv_scale_y = 1.0f);

    void draw() const;

    void destroy();

    ~MeshAsset();

    int32_t index_count() const;
    AssetID material_id() const;
    const AABB& local_aabb() const;

protected:
    std::ostream& print(std::ostream& os) const override;

private:
    std::string m_name;
    VertexFormat m_format;
    uint32_t m_vertices_num;
    uint32_t m_indices_num;
    AssetID m_material_id;

    AABB m_local_aabb;

    uint32_t m_vao = 0;
    uint32_t m_vbo = 0;
    uint32_t m_ebo = 0;

    void compute_local_aabb(const void* vertices);
    void upload(const void* vertices, const void* indices);
};
