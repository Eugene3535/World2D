#include <cassert>

#include "game/assets/AssetManager.hpp"


static AssetManager* sInstance;


AssetManager::AssetManager(GlResourceManager& glResources) noexcept :
    m_glResources(&glResources)
{
    assert(sInstance == nullptr);
    sInstance = this;
}


AssetManager::~AssetManager()
{
}


AssetManager* AssetManager::getInstance() noexcept
{
    return sInstance;
}