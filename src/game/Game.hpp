#pragma once

#include <vector>
#include <utility>

#include <SFML/System/Time.hpp>
#include <SFML/Graphics/RenderWindow.hpp>

#include "common/action/ActionManager.hpp"


struct System
{
    void (*execute)(void* /* Scene* */, const sf::Time) = nullptr;
    uint32_t frequency = 0;
    bool isEnabled = false;
};


class Game final
{
public:
    enum State
    {
        TitleScreen,
        Stage
    };

    Game() noexcept;
    ~Game();

    bool initialize() noexcept; // заменить на Context.load(game); Context - состояние всего приложения
    void update(const sf::Time deltaTime) noexcept; // вынести в UiController

    std::vector<std::pair<size_t(*)(void*, const sf::Time), void*>> m_longActions; // { .first = action, .second = data }

    std::vector<System> systems; // функции для обновления сцен, каждая сцена через свап вставляет свои системы

    sf::RenderWindow window;
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