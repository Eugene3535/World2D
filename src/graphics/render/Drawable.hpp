#pragma once

class Drawable
{
public:
    virtual ~Drawable() = default;
    virtual void draw(unsigned int shaderProgram) const noexcept = 0;
};