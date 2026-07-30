#pragma once

#include <SFML/System/Time.hpp>
#include <SFML/Graphics/View.hpp>

#include "files/assets/AssetManager.hpp"
#include "common/action/ActionManager.hpp"


struct System
{
    void (*execute)(void* /* Scene* */, const sf::Time) = nullptr;
    uint32_t frequency = 1;
    bool isEnabled = true;
};


struct Game
{
    enum State
    {
        TitleScreen,
        Stage
    };

    void update(const sf::Time deltaTime) noexcept; // вынести в UiController

    AssetManager assets;

    std::vector<std::pair<size_t(*)(void*, const sf::Time), void*>> m_longActions; // { .first = action, .second = data }

    std::vector<System> systems;


    sf::View view;
};


// void Action::update(float dt) noexcept // пример как итерировать по действиям
// {
//     for(auto system : m_systems)
//         system(this, dt);

//     auto quick_remove_at = [](auto& v, size_t idx)
//     {    
//         if (idx < v.size()) 
//             v[idx] = v.back();
        
//         v.pop_back();
//     };
    
//     for (size_t i = 0; i < m_actions.size(); ++i)
//     {
//         auto action = m_actions[i];
//         auto data = m_actionData[i];
//         size_t result = action(data, dt);

//         if(result)
//         {
//             m_actionAllocator.release(data, result);
//             quick_remove_at(m_actions, i);
//             quick_remove_at(m_actionData, i);
//         }
//     }
// }