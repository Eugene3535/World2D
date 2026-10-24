#pragma once

#include <entt/entt.hpp>


class World final
{
public:
    World() noexcept;
    ~World();

    bool open() noexcept;

private:
    entt::registry m_registry;
};