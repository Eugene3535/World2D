#pragma once

class Context
{
public:
    Context() noexcept;
    virtual ~Context();

    bool initOpenGL() noexcept;

private:
    void initDebug() noexcept;
};