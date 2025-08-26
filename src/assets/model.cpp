#include "assets/model.h"
#include "core/log.h"

#include <iostream>
#include <cstring>
#include <limits>
#include <cstdint>
#include <filesystem>

#include <assimp/Importer.hpp>
#include <assimp/scene.h>
#include <assimp/postprocess.h>

Model::Model(std::string name) : m_name(name.empty() ? "unnamed_model" : std::move(name)) {
}

Model::Model(std::string name, AssetID mesh_id) : Model(std::move(name)) {
    m_meshes.push_back(mesh_id);
}

std::shared_ptr<Model> Model::create_fallback() {
    auto model = std::make_shared<Model>("fallback_model");
    model->add_mesh(AssetManager::instance().get_fallback_id<Mesh>());
    return model;
}

std::optional<std::shared_ptr<Model>> Model::load_from_file(const std::string& path, const std::string& name) {
    Assimp::Importer importer;
    const aiScene* scene =
        importer.ReadFile(path, aiProcess_Triangulate | aiProcess_GenSmoothNormals | aiProcess_CalcTangentSpace |
                                    aiProcess_FlipUVs | aiProcess_JoinIdenticalVertices);

    if (!scene || scene->mFlags & AI_SCENE_FLAGS_INCOMPLETE || !scene->mRootNode) {
        ERR("[Model] Assimp load error: " << importer.GetErrorString())
        return std::nullopt;
    }

    auto model = std::make_shared<Model>(std::string(name));

    // Find the directory since "path" is actually the path of the model data file
    std::filesystem::path path_fs(std::move(path));

    // Append again the last separator
    model->m_directory = path_fs.parent_path().string() + "/";

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
        AssetManager& am = AssetManager::instance();
        bool loaded_material = false;
        AssetID mat_id;
        if (scene->HasMaterials()) {
            const aiMaterial* ai_mat = scene->mMaterials[ai_mesh->mMaterialIndex];
            mat_id = am.add_asset<Material>(ai_mat, model->m_directory, "full");
            loaded_material = true;
        }

        if (!loaded_material) {
            mat_id = am.get_fallback_id<Material>();
        }

        // Create mesh
        AssetID mesh_id = am.add_asset<Mesh>(std::move(ai_mesh->mName.C_Str()), VertexFormat::POS_NOR_TEX,
                                             vertices.data(), vertices.size(), indices.data(), indices.size(), mat_id);

        model->m_meshes.push_back(mesh_id);
    }

    return model;
}

const std::string& Model::directory() const {
    return m_directory;
}

const std::string& Model::name() const {
    return m_name;
}

void Model::add_mesh(AssetID mesh_id) {
    m_meshes.push_back(mesh_id);
}

const std::vector<AssetID>& Model::meshes() const {
    return m_meshes;
}

std::ostream& Model::print(std::ostream& os) const {
    return os << "Model(name: " << m_name << ", meshes_num: " << m_meshes.size() << ", directory: " << m_directory
              << ")";
}
