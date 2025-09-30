#pragma once

#include "assets/interfaces.h"
#include "assets/assets.h"

#include <unordered_map>
#include <memory>
#include <stdexcept>
#include <optional>
#include <typeindex>
#include <iostream>
#include <sstream>

template <typename T, typename U>
concept has_create_fallback_of = requires(AssetManager& am) {
    { T::create_fallback(am) } -> std::convertible_to<std::shared_ptr<U>>;
};

class AssetManager {
public:
    template <typename T>
        requires std::is_base_of_v<IAsset, T>
    AssetID add(std::shared_ptr<T> asset) {
        AssetID id = m_next_id++;
        m_assets[id] = std::move(asset);
        return id;
    }

    template <typename T, typename... Args>
        requires std::is_base_of_v<IAsset, T>
    AssetID add(Args&&... args) {
        auto asset = std::make_shared<T>(std::forward<Args>(args)...);
        return add<T>(std::move(asset));
    }

    bool exists(AssetID id) {
        auto it = m_assets.find(id);
        return it != m_assets.end();
    }

    template <typename T>
        requires std::is_base_of_v<IAsset, T>
    T& get(AssetID asset_id) {
        auto it = m_assets.find(asset_id);
        std::shared_ptr<IAsset>& asset_ptr = (it == m_assets.end()) ? m_assets.at(get_fallback_id<T>()) : it->second;
        return *static_cast<T*>(asset_ptr.get());
    }

    AssetID is_loaded(const std::string& path) {
        AssetID id = INVALID_ASSET;
        auto it = m_loaded_assets.find(path);
        if (it != m_loaded_assets.end()) {
            id = it->second;
        }

        return id;
    }

    void add_loaded(std::string path, AssetID id) {
        m_loaded_assets[std::move(path)] = id;
    }

    template <typename T>
        requires has_create_fallback_of<AssetCreator<T>, T>
    AssetID get_fallback_id() {
        std::type_index i(typeid(T));
        if (m_fallbacks.contains(i)) {
            return m_fallbacks[i];
        }

        auto fallback = AssetCreator<T>::create_fallback(*this);
        AssetID fallback_id = add<T>(std::move(fallback));
        auto [it, _] = m_fallbacks.try_emplace(i, fallback_id);
        return it->second;
    }

    template <typename T>
    AssetCreator<T> create(std::string name) {
        return AssetCreator<T>(*this, std::move(name));
    }

    template <typename T>
    AssetLoader<T> load(std::string name, std::string path, bool is_path_relative = true) {
        return AssetLoader<T>(*this, std::move(name), std::move(path), is_path_relative);
    }

    void set_last_used_shader(AssetID shader_id) {
        m_last_used_shader = shader_id;
    }

    AssetID last_used_shader() {
        return m_last_used_shader;
    }

    void print(AssetID asset_id) {
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

private:
    AssetID m_next_id = 1;
    std::unordered_map<AssetID, std::shared_ptr<IAsset>> m_assets;
    std::unordered_map<std::string, AssetID> m_loaded_assets;
    std::unordered_map<std::type_index, AssetID> m_fallbacks;
    AssetID m_last_used_shader = INVALID_ASSET;
};

#include "assets/interfaces_impl.tpp"