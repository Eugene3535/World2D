#pragma once


namespace sf
{
    class RenderWindow;
}


class Context final
{
public:
    Context(class sf::RenderWindow& rw) noexcept;
    ~Context();

    bool load(struct Game& game) noexcept;
    void save(struct Game& game) noexcept;

private:
    class sf::RenderWindow& m_window;
};