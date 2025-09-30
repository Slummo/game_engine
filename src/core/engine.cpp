#include "core/engine.h"
#include "managers/entity_manager.h"
#include "managers/system_manager.h"
#include "managers/context_manager.h"
#include "managers/asset_manager.h"

Engine::Engine() {
    m_em = std::make_unique<EntityManager>();
    m_sm = std::make_unique<SystemManager>();
    m_cm = std::make_unique<ContextManager>();
    m_am = std::make_unique<AssetManager>();
}

Engine::~Engine() = default;