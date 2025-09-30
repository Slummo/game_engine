#include "assets/model_asset.h"
#include "assets/mesh_asset.h"
#include "assets/material_asset.h"
#include "assets/shader_asset.h"
#include "managers/asset_manager.h"

#include <cstring>
#include <limits>
#include <filesystem>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

ModelAsset::ModelAsset(std::string name, std::string directory)
    : IAsset(name.empty() ? "unnamed_model" : std::move(name)), m_directory(std::move(directory)) {
}

void ModelAsset::draw(AssetManager& am, Transform& tr, Camera& cam, Light& light) {
    for (AssetID mesh_id : m_meshes) {
        MeshAsset& mesh = am.get<MeshAsset>(mesh_id);
        MaterialAsset& mat = am.get<MaterialAsset>(mesh.material_id());

        AssetID shader_id = mat.shader_id();
        ShaderAsset& shader = am.get<ShaderAsset>(shader_id);
        if (shader_id != am.last_used_shader()) {
            shader.use();
            am.set_last_used_shader(shader_id);
        }

        // Vertex shader
        const glm::mat4 model_mat = tr.model_matrix();
        glm::mat3 normal_mat = glm::transpose(glm::inverse(glm::mat3(model_mat)));
        shader.set_matrix_4f("Projection", cam.proj_matrix());
        shader.set_matrix_4f("View", cam.view_matrix());
        shader.set_matrix_4f("Model", model_mat);
        shader.set_matrix_3f("Normal", normal_mat);

        // Fragment shader
        glm::vec3 base_color = mat.get_param_or_default<glm::vec3>("base_color", glm::vec3(1.0f));  // default white
        shader.set_vec_3f("mat.base_color", base_color);

        shader.set_bool("mat.has_diffuse_map", false);
        shader.set_bool("mat.has_specular_map", false);
        shader.set_bool("mat.has_ambient_map", false);

        AssetID tex_id;
        if (mat.get_texture(MaterialTextureType::Diffuse, tex_id)) {  // diffuse texture
            TextureAsset& diffuse_tex = am.get<TextureAsset>(tex_id);
            diffuse_tex.bind(0);
            shader.set_int("mat.diffuse_map", 0);
            shader.set_bool("mat.has_diffuse_map", true);
        }
        if (mat.get_texture(MaterialTextureType::Specular, tex_id)) {  // specular texture
            TextureAsset& specular_tex = am.get<TextureAsset>(tex_id);
            specular_tex.bind(1);
            shader.set_int("mat.specular_map", 1);
            shader.set_bool("mat.has_specular_map", true);
        }
        if (mat.get_texture(MaterialTextureType::Ambient, tex_id)) {  // ambient texture
            TextureAsset& ambient_tex = am.get<TextureAsset>(tex_id);
            ambient_tex.bind(2);
            shader.set_int("mat.ambient_map", 2);
            shader.set_bool("mat.has_ambient_map", true);
        }

        float shininess = mat.get_param_or_default<float>("shininess", 32.0f);
        shader.set_float("mat.shininess", shininess);

        shader.set_vec_3f("light.direction", light.direction);
        shader.set_vec_3f("light.color", light.color);
        shader.set_float("light.intensity", light.intensity);
        shader.set_bool("light.is_directional", true);

        shader.set_vec_3f("camera_world_pos", cam.world_position());

        mesh.draw();

        TextureAsset::unbind();
    }
}

const std::string& ModelAsset::directory() const {
    return m_directory;
}

void ModelAsset::add_mesh(AssetID mesh_id) {
    m_meshes.push_back(mesh_id);
}

const std::vector<AssetID>& ModelAsset::meshes() const {
    return m_meshes;
}

std::ostream& ModelAsset::print(std::ostream& os) const {
    return os << "ModelAsset(name: " << m_name << ", meshes_num: " << m_meshes.size() << ", directory: " << m_directory
              << ")";
}

std::shared_ptr<ModelAsset> AssetCreator<ModelAsset>::create_fallback(AssetManager& am) {
    AssetID fallback_mesh = am.get_fallback_id<MeshAsset>();
    auto model = std::make_shared<ModelAsset>("fallback_model", "");
    model->add_mesh(fallback_mesh);
    return model;
}

AssetID AssetCreator<ModelAsset>::finish() {
    auto model = std::make_shared<ModelAsset>(name, "");
    AssetID mesh = resolve_slot(ModelDepSlot::Mesh, am.get_fallback_id<MeshAsset>());
    model->add_mesh(mesh);
    return am.add<ModelAsset>(std::move(model));
}

const char* AssetLoader<ModelAsset>::base_path() {
    return "assets/models/";
}

AssetID AssetLoader<ModelAsset>::finish() {
    AssetID id = am.is_loaded(absolute_path);
    if (id != INVALID_ASSET) {
        return id;
    }

    Assimp::Importer importer;
    const aiScene* scene = importer.ReadFile(absolute_path, aiProcess_Triangulate | aiProcess_GenSmoothNormals |
                                                                aiProcess_CalcTangentSpace | aiProcess_FlipUVs |
                                                                aiProcess_JoinIdenticalVertices);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        ERR("[AssetLoader<ModelAsset>] Assimp load error: " << importer.GetErrorString())
        return INVALID_ASSET;
    }

    // Find the directory since "path" is actually the path of the model data file
    std::filesystem::path path_fs(absolute_path);
    std::string dir(path_fs.parent_path().string() + "/");  // Append again the last separator

    auto model = std::make_shared<ModelAsset>(name, dir);

    // Iterate over meshes
    for (uint32_t m = 0; m < scene->mNumMeshes; m++) {
        aiMesh* ai_mesh = scene->mMeshes[m];
        std::vector<Vertex_PNT> vertices;
        vertices.reserve(ai_mesh->mNumVertices);

        // Populate vertices
        for (uint32_t i = 0; i < ai_mesh->mNumVertices; i++) {
            Vertex_PNT v{};
            v.pos3 = glm::vec3(ai_mesh->mVertices[i].x, ai_mesh->mVertices[i].y, ai_mesh->mVertices[i].z);
            if (ai_mesh->HasNormals()) {
                v.nor3 = glm::vec3(ai_mesh->mNormals[i].x, ai_mesh->mNormals[i].y, ai_mesh->mNormals[i].z);
            } else {
                v.nor3 = glm::vec3(0.0f, 1.0f, 0.0f);
            }
            if (ai_mesh->mTextureCoords[0]) {
                v.tex2 = glm::vec2(ai_mesh->mTextureCoords[0][i].x, ai_mesh->mTextureCoords[0][i].y);
            } else {
                v.tex2 = glm::vec2(0.0f, 0.0f);
            }
            vertices.push_back(v);
        }

        std::vector<uint32_t> indices;
        indices.reserve(ai_mesh->mNumFaces * 3);

        // Populate indices
        for (uint32_t i = 0; i < ai_mesh->mNumFaces; i++) {
            aiFace face = ai_mesh->mFaces[i];
            if (face.mNumIndices != 3) {
                continue;  // should not happen after triangulation
            }
            indices.push_back(face.mIndices[0]);
            indices.push_back(face.mIndices[1]);
            indices.push_back(face.mIndices[2]);
        }

        // Load material if present
        AssetID mat_id = INVALID_ASSET;
        if (scene->HasMaterials()) {
            const aiMaterial* ai_mat = scene->mMaterials[ai_mesh->mMaterialIndex];
            std::vector<TexData> tex_data = MaterialAsset::load_textures(
                ai_mat, std::move(dir),
                {MaterialTextureType::Ambient, MaterialTextureType::Diffuse, MaterialTextureType::Specular});

            std::string mat_name = MaterialAsset::load_name(ai_mat);

            auto creator = am.create<MaterialAsset>(mat_name);
            creator.add_dep(MaterialDepSlot::Shader, LoadDep(LoadableAssetType::Shader, "full", "full"));

            for (const auto& t : tex_data) {
                switch (t.type) {
                    case MaterialTextureType::Diffuse: {
                        creator.add_dep(MaterialDepSlot::Diffuse,
                                        LoadDep(LoadableAssetType::Texture, mat_name + "_diffuse_tex", t.path, false));
                        break;
                    }
                    case MaterialTextureType::Specular: {
                        creator.add_dep(MaterialDepSlot::Specular,
                                        LoadDep(LoadableAssetType::Texture, mat_name + "_specular_tex", t.path, false));
                        break;
                    }
                    case MaterialTextureType::Ambient: {
                        creator.add_dep(MaterialDepSlot::Ambient,
                                        LoadDep(LoadableAssetType::Texture, mat_name + "_ambient_tex", t.path, false));
                        break;
                    }
                    default: {
                        ERR("[AssetLoader<ModelAsset>] Unknown texture type!");
                        break;
                    }
                }
            }

            mat_id = creator.finish();
        }

        if (mat_id == INVALID_ASSET) {
            mat_id = am.get_fallback_id<MaterialAsset>();
        }

        // Create mesh
        AssetID mesh_id = am.create<MeshAsset>(ai_mesh->mName.C_Str())
                              .set_mesh_type(MeshType::CUSTOM)
                              .set_vertex_format(VertexFormat::POS_NOR_TEX)
                              .set_data(vertices.data(), vertices.size(), indices.data(), indices.size())
                              .add_dep(MeshDepSlot::Material, CreateDep{mat_id})
                              .finish();

        model->add_mesh(mesh_id);
    }

    id = am.add<ModelAsset>(std::move(model));
    am.add_loaded(std::move(absolute_path), id);
    return id;
}