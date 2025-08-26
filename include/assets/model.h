#pragma once

#include "managers/asset_manager.h"
#include "core/types.h"

#include <vector>
#include <glm/glm.hpp>
#include <string>
#include <memory>
#include <optional>
#include <unordered_map>

class Model : public IAsset {
public:
    Model() = delete;

    // Used when loading model from file
    Model(std::string name);

    // Used when creating a model with a single mesh
    Model(std::string name, AssetID mesh_id);

    static std::shared_ptr<Model> create_fallback();

    static std::optional<std::shared_ptr<Model>> load_from_file(const std::string& path, const std::string& name);
    static const char* base_path() {
        return "assets/models/";
    }

    const std::string& name() const;
    const std::string& directory() const;

    void add_mesh(AssetID mesh_id);
    const std::vector<AssetID>& meshes() const;

protected:
    std::ostream& print(std::ostream& os) const override;

private:
    std::string m_directory;
    std::string m_name;
    std::vector<AssetID> m_meshes;
};
