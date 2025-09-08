#pragma once

#include "assets/iasset.h"
#include "assets/assets.h"
#include "core/log.h"
#include "core/types/type_name.h"

#include <unordered_map>
#include <memory>
#include <string>
#include <stdexcept>
#include <optional>
#include <typeindex>
#include <iostream>
#include <sstream>

template <typename T, typename... Args>
concept is_loadable = requires(const std::string& path, Args&&... args) {
    { T::load_from_file(path, args...) } -> std::convertible_to<std::optional<std::shared_ptr<T>>>;
} && requires() {
    { T::base_path() } -> std::convertible_to<const char*>;
};

template <typename T>
concept has_fallback = requires {
    { T::create_fallback() } -> std::convertible_to<std::shared_ptr<T>>;
};

class AssetManager {
public:
    static AssetManager& instance() {
        static AssetManager singleton;
        return singleton;
    }

    template <typename T, typename... Args>
        requires std::is_base_of_v<IAsset, T>
    AssetID add_asset(Args&&... args) {
        AssetID id = m_next_id++;
        m_assets[id] = std::make_shared<T>(std::forward<Args>(args)...);
        return id;
    }

    template <typename T>
        requires std::is_base_of_v<IAsset, T>
    AssetID add_asset(std::shared_ptr<T> asset) {
        AssetID id = m_next_id++;
        m_assets[id] = std::move(asset);
        return id;
    }

    template <typename T, typename... Args>
        requires std::is_base_of_v<IAsset, T> && is_loadable<T, Args...> && has_fallback<T>
    AssetID load_asset(const std::string& name, Args&&... args) {
        std::string path(T::base_path() + name);

        auto it = m_loaded_assets.find(path);
        if (it != m_loaded_assets.end()) {
            // Return an already loaded asset
            return it->second;
        }

        auto opt = T::load_from_file(path, std::forward<Args>(args)...);
        if (!opt) {
            ERR("[AssetManager] Failed to load asset of type " << readable_type_name<T>() << " from path: " << path);
            return get_or_create_fallback<T>();
        }

        AssetID id = add_asset<T>(std::move(*opt));
        // Add the loaded asset to the map
        m_loaded_assets[std::move(path)] = id;
        return id;
    }

    template <typename T, typename... Args>
        requires std::is_base_of_v<IAsset, T> && is_loadable<T, Args...> && has_fallback<T>
    AssetID load_asset_path(const std::string& path, Args&&... args) {
        auto it = m_loaded_assets.find(path);
        if (it != m_loaded_assets.end()) {
            // Return an already loaded asset
            return it->second;
        }

        auto opt = T::load_from_file(path, std::forward<Args>(args)...);
        if (!opt) {
            ERR("[AssetManager] Failed to load asset of type " << readable_type_name<T>() << " from path: " << path);
            return get_or_create_fallback<T>();
        }

        AssetID id = add_asset<T>(std::move(*opt));
        // Add the loaded asset to the map
        m_loaded_assets[std::move(path)] = id;
        return id;
    }

    template <typename T>
        requires std::is_base_of_v<IAsset, T> && has_fallback<T>
    T& get_asset(AssetID asset_id) {
        auto it = m_assets.find(asset_id);
        std::shared_ptr<IAsset>& asset_ptr =
            (it == m_assets.end()) ? m_assets.at(get_or_create_fallback<T>()) : it->second;
        return *static_cast<T*>(asset_ptr.get());
    }

    template <typename T>
        requires std::is_base_of_v<IAsset, T> && has_fallback<T>
    AssetID get_fallback_id() {
        return get_or_create_fallback<T>();
    }

    void set_last_used_shader(AssetID shader_id) {
        m_last_used_shader = shader_id;
    }

    AssetID last_used_shader() {
        return m_last_used_shader;
    }

    void print_asset(AssetID asset_id) {
        auto it = m_assets.find(asset_id);
        if (it == m_assets.end()) {
            ERR("[AssetManager] asset with id " << asset_id << " not found");
            return;
        }

        LOG(*it->second);
    }

    std::string asset_to_string(AssetID asset_id) {
        auto it = m_assets.find(asset_id);
        if (it == m_assets.end()) {
            ERR("[AssetManager] asset with id " << asset_id << " not found");
            return "";
        }

        std::stringstream ss;
        ss << *it->second;
        return ss.str();
    }

    // Disable copies
    AssetManager(const AssetManager&) = delete;
    AssetManager& operator=(const AssetManager&) = delete;

private:
    AssetID m_next_id = 0;
    std::unordered_map<AssetID, std::shared_ptr<IAsset>> m_assets;
    std::unordered_map<std::string, AssetID> m_loaded_assets;
    std::unordered_map<std::type_index, AssetID> m_fallbacks;
    AssetID m_last_used_shader;

    AssetManager() = default;

    template <typename T>
    AssetID add_fallback() {
        auto fallback = T::create_fallback();
        return add_asset<T>(std::move(fallback));
    }

    template <typename T>
    AssetID get_or_create_fallback() {
        std::type_index i(typeid(T));

        auto [it, _] = m_fallbacks.try_emplace(i, add_fallback<T>());
        return it->second;
    }
};
