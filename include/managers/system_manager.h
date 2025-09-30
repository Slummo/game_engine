#pragma once

#include "systems/isystem.h"
#include "systems/systems.h"
#include "core/types/type_name.h"
#include "core/log.h"

#include <unordered_map>
#include <typeindex>
#include <memory>
#include <stdexcept>
#include <format>

class Engine;

class SystemManager {
public:
    template <typename T, typename... Args>
        requires std::is_base_of_v<ISystem, T>
    void add(Args&&... args) {
        std::type_index i(typeid(T));
        auto [it, added] = m_systems.try_emplace(i, std::make_unique<T>(std::forward<Args>(args)...));
        if (added) {
            LOG("[SystemManager] Added " << readable_type_name<T>());
        }
    }

    template <typename T>
        requires std::is_base_of_v<ISystem, T>
    T& get() {
        std::type_index i(typeid(T));
        if (!m_systems.contains(i)) {
            throw std::runtime_error(
                std::format("[SystemManager] Trying to fetch {} which wasn't added!", readable_type_name<T>()));
        }

        return *static_cast<T*>(m_systems.at(i).get());
    }

    void init_all(Engine& engine) {
        for (auto& [i, s] : m_systems) {
            s->init(engine);
        }
    }

    void update_all(Engine& engine) {
        for (auto& [i, s] : m_systems) {
            s->update(engine);
        }
    }

    void shutdown_all(Engine& engine) {
        for (auto& [i, s] : m_systems) {
            s->shutdown(engine);
        }
        m_systems.clear();
    }

private:
    std::unordered_map<std::type_index, std::unique_ptr<ISystem>> m_systems;
};
