#pragma once

class Context final
{
public:
    bool load(struct Game& game) noexcept;
    void save(struct Game& game) noexcept;
};