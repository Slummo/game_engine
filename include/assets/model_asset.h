#pragma once

#include "assets/interfaces.h"
#include "components/transform.h"
#include "components/camera.h"
#include "components/light.h"

#include <vector>
#include <glm/glm.hpp>
#include <string>
#include <memory>
#include <optional>
#include <unordered_map>

class ModelAsset : public IAsset {
public:
    ModelAsset(std::string name, std::string directory);

    void draw(AssetManager& am, Transform& tr, Camera& cam, Light& light);

    const std::string& directory() const;

    void add_mesh(AssetID mesh_id);
    const std::vector<AssetID>& meshes() const;

protected:
    std::ostream& print(std::ostream& os) const override;

private:
    std::string m_directory;
    std::vector<AssetID> m_meshes;
};

enum class ModelDepSlot { Mesh };

template <>
class AssetCreator<ModelAsset> : public AssetCreatorDep<ModelAsset, ModelDepSlot> {
public:
    using Base = AssetCreatorDep<ModelAsset, ModelDepSlot>;
    using Base::Base;

    static std::shared_ptr<ModelAsset> create_fallback(AssetManager& am);
    AssetID finish() override;
};

template <>
class AssetLoader<ModelAsset> : public AssetLoaderNoDep<ModelAsset> {
public:
    using Base = AssetLoaderNoDep<ModelAsset>;
    using Base::Base;

    static const char* base_path();
    AssetID finish();
};