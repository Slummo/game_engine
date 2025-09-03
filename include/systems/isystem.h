#pragma once

#include "managers/entity_manager.h"
#include "managers/context_manager.h"
#include "components/components.h"

#include <string>
#include <cstdint>

class ISystemBase {
public:
    virtual ~ISystemBase() = default;

    virtual void init(EntityManager&, ContextManager&) = 0;
    virtual void update(EntityManager&, ContextManager&) = 0;
    virtual void shutdown(EntityManager&, ContextManager&) = 0;
};

template <typename... Contexts>
    requires((std::is_base_of_v<IContext, Contexts> && ...))
class ISystem : public ISystemBase {
public:
    virtual ~ISystem() = default;

    virtual void init(EntityManager&, Contexts&...) {
    }
    virtual void update(EntityManager&, Contexts&...) = 0;
    virtual void shutdown(EntityManager&, Contexts&...) {
    }

    void init(EntityManager& em, ContextManager& cm) final {
        init(em, cm.get_context<Contexts>()...);
    }

    void update(EntityManager& em, ContextManager& cm) final {
        update(em, cm.get_context<Contexts>()...);
    }

    void shutdown(EntityManager& em, ContextManager& cm) final {
        shutdown(em, cm.get_context<Contexts>()...);
    }
};