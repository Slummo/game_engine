#pragma once

#include "contexts/icontext.h"
#include "contexts/contexts.h"
#include "core/types/type_name.h"
#include "core/log.h"

#include <unordered_map>
#include <typeindex>
#include <memory>
#include <stdexcept>
#include <format>

class ContextManager {
public:
    template <typename T, typename... Args>
        requires std::is_base_of_v<IContext, T>
    T& add(Args&&... args) {
        std::type_index i(typeid(T));
        auto [it, added] = m_contexts.try_emplace(i, std::make_unique<T>(std::forward<Args>(args)...));
        if (added) {
            LOG("[ContextManager] Added " << readable_type_name<T>());
        }

        return *static_cast<T*>(it->second.get());
    }

    template <typename T>
        requires std::is_base_of_v<IContext, T>
    T& get() {
        std::type_index i(typeid(T));
        if (!m_contexts.contains(i)) {
            throw std::runtime_error(
                std::format("[ContextManager] Trying to fetch {} which wasn't added!", readable_type_name<T>()));
        }

        return *static_cast<T*>(m_contexts.at(i).get());
    }

private:
    std::unordered_map<std::type_index, std::unique_ptr<IContext>> m_contexts;
};