#include "game/world/Location.hpp"


Location::Location(GlResourceManager& resources) noexcept :
    m_glResources(&resources),
    m_tilemap(m_loader, resources)
{
}

Location::~Location()
{
}