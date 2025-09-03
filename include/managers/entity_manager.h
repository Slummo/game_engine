#pragma once

#include "core/types/id.h"
#include "components/icomponent.h"

#include <unordered_map>
#include <memory>
#include <stdexcept>
#include <typeindex>
#include <vector>
#include <ranges>
#include <string>

class EntityManager {
public:
    EntityID create_entity() {
        EntityID id;

        // Reuse previously removed entities
        if (!m_free_ids.empty()) {
            id = m_free_ids.back();
            m_free_ids.pop_back();
        } else {
            id = m_next_id++;
        }

        return id;
    }

    void destroy_entity(EntityID entity_id) {
        // Remove all components
        for (auto& [_, pool_ptr] : m_pools) {
            auto pool = dynamic_cast<IComponentPool*>(pool_ptr.get());
            pool->remove_component(entity_id);
        }

        m_free_ids.push_back(entity_id);
    }

    // Attach a certain component to an entity
    template <typename T, typename... Args>
        requires std::is_base_of_v<IComponent, T>
    T& add_component(EntityID entity_id, Args&&... args) {
        return get_pool<T>().add_component(entity_id, std::forward<Args>(args)...);
    }

    // Adds multiple components that don't have arguments in their constructors
    template <typename... Components>
        requires(std::is_base_of_v<IComponent, Components> && ...)
    void add_components(EntityID entity_id) {
        (add_component<Components>(entity_id), ...);
    }

    // Checks if an entity has a certain component attached
    template <typename T>
        requires std::is_base_of_v<IComponent, T>
    bool has_component(EntityID entity_id) {
        return get_pool<T>().has_component(entity_id);
    }

    // Checks if an entity has certain components attached
    template <typename... Components>
        requires(std::is_base_of_v<IComponent, Components> && ...)
    bool has_components(EntityID entity_id) {
        return (has_component<Components>(entity_id) && ...);
    }

    // Returns the certain component attached to the entity. Might throw
    template <typename T>
        requires std::is_base_of_v<IComponent, T>
    T& get_component(EntityID entity_id) {
        return get_pool<T>().get_component(entity_id);
    }

    // Returns the certain components attached to the entity. Might throw
    template <typename... Components>
        requires(std::is_base_of_v<IComponent, Components> && ...)
    auto get_components(EntityID entity_id) {
        return std::tie(get_component<Components>(entity_id)...);
    }

    // Removes the certain component attached to the entity. Might throw
    template <typename T>
        requires std::is_base_of_v<IComponent, T>
    void remove_component(EntityID entity_id) {
        get_pool<T>().remove_component(entity_id);
    }

    // Removes the certain components attached to the entity. Might throw
    template <typename... Components>
        requires(std::is_base_of_v<IComponent, Components> && ...)
    void remove_components(EntityID entity_id) {
        (remove_component<Components>(entity_id), ...);
    }

    template <typename... Components>
        requires((std::is_base_of_v<IComponent, Components> && ...))
    auto entities_with() {
        // Get the pool relative to the first component of type T
        using T = std::tuple_element_t<0, std::tuple<Components...>>;
        ComponentPool<T>& base_pool = get_pool<T>();

        // Get the entities with the first component of type T
        auto base_entities = base_pool.components | std::views::keys;

        // Filter entities that have all components
        auto filtered = base_entities | std::views::filter([this](EntityID e) {
                            return (get_pool<Components>().has_component(e) && ...);
                        });

        // Concatenate tuple to return EntityID, Component&...
        return filtered | std::views::transform([this](EntityID e) {
                   return std::tuple_cat(std::make_tuple(e),
                                         std::forward_as_tuple(get_pool<Components>().get_component(e)...));
               });
    }

private:
    struct IComponentPool {
        virtual ~IComponentPool() = default;
        virtual void remove_component(EntityID entity_id) = 0;
    };

    template <typename T>
        requires std::is_base_of_v<IComponent, T>
    struct ComponentPool : public IComponentPool {
        std::unordered_map<EntityID, T> components;

        template <typename... Args>
        T& add_component(EntityID entity_id, Args&&... args) {
            auto [it, _] = components.try_emplace(entity_id, std::forward<Args>(args)...);
            return it->second;
        }

        bool has_component(EntityID entity_id) {
            return components.contains(entity_id);
        }

        T& get_component(EntityID entity_id) {
            if (!has_component(entity_id)) {
                throw std::runtime_error("[EntityManager] An entity doesnt have the required component!");
            }

            return components.at(entity_id);
        }

        void remove_component(EntityID entity_id) {
            components.erase(entity_id);
        }
    };

    std::unordered_map<std::type_index, std::unique_ptr<IComponentPool>> m_pools;
    EntityID m_next_id = 0;
    std::vector<EntityID> m_free_ids;

    template <typename T>
    ComponentPool<T>& get_pool() {
        std::type_index i(typeid(T));
        auto [it, _] = m_pools.try_emplace(i, std::make_unique<ComponentPool<T>>());
        IComponentPool* pool = it->second.get();
        auto* concrete_pool = static_cast<ComponentPool<T>*>(pool);
        if (!concrete_pool) {
            throw std::runtime_error("NULLPTR");
        }

        return *concrete_pool;
    }
};
