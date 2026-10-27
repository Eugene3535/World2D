#pragma once

#include <entt/entt.hpp>


class World final
{
public:
    World() noexcept;
    ~World();

    bool open() noexcept;

    static World* getInstance() noexcept;

private:
    entt::registry m_registry;
};