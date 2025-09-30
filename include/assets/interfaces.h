#pragma once

#include "core/log.h"
#include "core/types/id.h"

#include <string>
#include <array>
#include <variant>
#include <vector>
#include <stdexcept>

class AssetManager;

class IAsset : public Printable {
public:
    IAsset(std::string name) : m_name(std::move(name)) {
    }

    const std::string& name() const {
        return m_name;
    }

    virtual ~IAsset() = default;

protected:
    std::string m_name;
};

struct CreateDep {
    AssetID id;
};
enum class LoadableAssetType { Sound, Shader, Texture, Model };
enum class TextureType { None, Diffuse, Specular, Ambient };
struct LoadDep {
    LoadDep(LoadableAssetType type, std::string name, std::string path, bool is_path_relative = true,
            TextureType tex_type = TextureType::None)
        : type(std::move(type)),
          name(std::move(name)),
          path(std::move(path)),
          tex_type(std::move(tex_type)),
          is_path_relative(is_path_relative) {
    }

    LoadableAssetType type;
    std::string name;
    std::string path;
    TextureType tex_type;
    bool is_path_relative;
};
using Dep = std::variant<std::monostate, CreateDep, LoadDep>;

template <typename T, typename Slot, typename Derived>
class AssetCreatorBase {
public:
    AssetCreatorBase(AssetManager& am, std::string name) : am(am), name(std::move(name)) {
    }

    Derived& add_dep(Slot slot, Dep dep) {
        auto i = static_cast<size_t>(slot);
        deps[i] = std::move(dep);
        return static_cast<Derived&>(*this);
    }

    virtual AssetID finish() = 0;

protected:
    AssetManager& am;
    std::string name;

    AssetID resolve_slot(Slot slot, AssetID fallback_id, bool required = true) {
        auto i = static_cast<size_t>(slot);
        try {
            return resolve_dep(deps[i], required);
        } catch (const std::exception& e) {
            ERR("[AssetCreatorBase] Error while resolving dependency for asset '" << name << "' in slot " << i << ": "
                                                                                  << e.what());
            return fallback_id;
        }
    }

private:
    std::array<Dep, 8> deps{};
    AssetID resolve_dep(const Dep& dep, bool required = true);
};

template <typename T, typename Derived>
class AssetCreatorBase<T, void, Derived> {
public:
    AssetCreatorBase(AssetManager& am, std::string name) : am(am), name(std::move(name)) {
    }

    virtual AssetID finish() = 0;

protected:
    AssetManager& am;
    std::string name;
};

template <typename T>
class AssetCreator;

template <typename T, typename Slot>
using AssetCreatorDep = AssetCreatorBase<T, Slot, AssetCreator<T>>;

template <typename T>
using AssetCreatorNoDep = AssetCreatorBase<T, void, AssetCreator<T>>;

template <typename T>
concept has_base_path = requires() {
    { T::base_path() } -> std::convertible_to<const char*>;
};

template <typename T, typename Slot, typename Derived>
class AssetLoaderBase : public AssetCreatorBase<T, Slot, Derived> {
public:
    AssetLoaderBase(AssetManager& am, std::string name, std::string path, bool is_path_relative = true)
        requires has_base_path<Derived>
        : AssetCreatorBase<T, Slot, Derived>(am, std::move(name)),
          absolute_path(is_path_relative ? std::string(Derived::base_path() + path) : std::move(path)) {
    }

protected:
    std::string absolute_path;
};

template <typename T>
class AssetLoader;

template <typename T, typename Slot>
using AssetLoaderDep = AssetLoaderBase<T, Slot, AssetLoader<T>>;

template <typename T>
using AssetLoaderNoDep = AssetLoaderBase<T, void, AssetLoader<T>>;