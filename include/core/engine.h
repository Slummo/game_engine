#pragma once

#include <memory>

class EntityManager;
class SystemManager;
class ContextManager;
class AssetManager;

struct Engine {
public:
    Engine();

    EntityManager& em() {
        return *m_em;
    }

    SystemManager& sm() {
        return *m_sm;
    }

    ContextManager& cm() {
        return *m_cm;
    }

    AssetManager& am() {
        return *m_am;
    }

    ~Engine();

private:
    std::unique_ptr<EntityManager> m_em;
    std::unique_ptr<SystemManager> m_sm;
    std::unique_ptr<ContextManager> m_cm;
    std::unique_ptr<AssetManager> m_am;
};