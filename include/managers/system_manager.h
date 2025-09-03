#pragma once

#include "systems/isystem.h"
#include "systems/systems.h"
#include "core/log.h"
#include "core/types/type_name.h"

#include <unordered_map>
#include <typeindex>
#include <memory>
#include <stdexcept>

class SystemManager {
public:
    template <typename T, typename... Args>
        requires std::is_base_of_v<ISystemBase, T>
    void add_system(Args&&... args) {
        std::type_index i(typeid(T));
        auto [it, added] = m_systems.try_emplace(i, std::make_unique<T>(std::forward<Args>(args)...));
        if (added) {
            LOG("[SystemManager] Added " << readable_type_name<T>());
        }
    }

    template <typename T>
        requires std::is_base_of_v<ISystemBase, T>
    T& get_system() {
        std::type_index i(typeid(T));
        if (!m_systems.contains(i)) {
            throw std::runtime_error(
                std::format("[SystemManager] Trying to fetch {} which wasn't added!", readable_type_name<T>()));
        }

        return *static_cast<T*>(m_systems.at(i).get());
    }

    void init_all(EntityManager& em, ContextManager& cm) {
        for (auto& [i, s] : m_systems) {
            s->init(em, cm);
        }
    }

    void update_all(EntityManager& em, ContextManager& cm) {
        for (auto& [i, s] : m_systems) {
            s->update(em, cm);
        }
        cm.get_context<EventContext>().dispatch();
    }

    void shutdown_all(EntityManager& em, ContextManager& cm) {
        for (auto& [i, s] : m_systems) {
            s->shutdown(em, cm);
        }
        m_systems.clear();
    }

private:
    std::unordered_map<std::type_index, std::unique_ptr<ISystemBase>> m_systems;
};
