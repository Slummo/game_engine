#pragma once

#include "contexts/icontext.h"
#include "contexts/contexts.h"

#include <unordered_map>
#include <typeindex>
#include <memory>
#include <stdexcept>

class ContextManager {
public:
    template <typename T, typename... Args>
        requires std::is_base_of_v<IContext, T>
    T& add_context(Args&&... args) {
        std::type_index i(typeid(T));
        auto [it, added] = m_contexts.try_emplace(i, std::make_unique<T>(std::forward<Args>(args)...));
        if (added) {
            LOG("[ContextManager] Added " << readable_type_name<T>());
        }

        return *static_cast<T*>(it->second.get());
    }

    template <typename T>
        requires std::is_base_of_v<IContext, T>
    T& get_context() {
        std::type_index i(typeid(T));
        auto* context = static_cast<T*>(m_contexts.at(i).get());
        if (!context) {
            throw std::runtime_error("[ContextManager] Trying to fetch a context that wasn't added!");
        }
        return *context;
    }

private:
    std::unordered_map<std::type_index, std::unique_ptr<IContext>> m_contexts;
};