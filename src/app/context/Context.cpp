#include <SFML/Graphics/RenderWindow.hpp>

#include "app/game/Game.hpp"
#include "app/context/Context.hpp"



Context::Context(sf::RenderWindow& rw) noexcept:
    m_window(rw)
{

}


Context::~Context() = default;


bool Context::load(Game& game) noexcept
{
    game.view = m_window.getView();

    return true;
}


void Context::save(Game& game) noexcept
{

}