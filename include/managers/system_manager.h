#pragma once

#include "systems/isystem.h"
#include "systems/systems.h"
#include "core/log.h"
#include "core/types.h"

#include <unordered_map>
#include <typeindex>
#include <memory>
#include <stdexcept>

class ECS;

class SystemManager {
public:
    SystemManager() = default;

    template <typename T, typename... Args>
        requires std::is_base_of_v<ISystem, T>
    void add_system(Args&&... args) {
        std::type_index i(typeid(T));
        auto [it, added] = m_systems.try_emplace(i, std::make_unique<T>(std::forward<Args>(args)...));
        if (added) {
            LOG("[SystemManager] Added " << readable_type_name<T>());
        }
    }

    template <typename T>
        requires std::is_base_of_v<ISystem, T>
    T& get_system() {
        std::type_index i(typeid(T));
        auto* system = static_cast<T*>(m_systems.at(i).get());
        if (!system) {
            std::runtime_error("NULLPTR");
        }
        return *system;
    }

    void init_all(ECS& ecs) {
        for (auto& [i, s] : m_systems) {
            s->init(ecs);
        }
    }

    // Variable-timestep update (per-frame)
    void update_all(ECS& ecs, float dt) {
        for (auto& [i, s] : m_systems) {
            s->update(ecs, dt);
        }
    }

    // Fixed-timestep update (called with fixed_dt possibly multiple times per frame)
    void fixed_update_all(ECS& ecs, float fixed_dt) {
        for (auto& [i, s] : m_systems) {
            s->fixed_update(ecs, fixed_dt);
        }
    }

    void shutdown_all(ECS& ecs) {
        for (auto& [i, s] : m_systems) {
            s->shutdown(ecs);
        }
        m_systems.clear();
    }

private:
    std::unordered_map<std::type_index, std::unique_ptr<ISystem>> m_systems;
};
