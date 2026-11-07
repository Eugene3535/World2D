#pragma once

#include <memory>

#include <cglm/struct/ivec2.h>
#include <cglm/struct/vec4.h>


class MainWindow final
{
public:
    MainWindow(struct WindowData& data) noexcept;
    ~MainWindow();

    bool open(const char* title, int width, int height) noexcept;
    void close() const noexcept;

    void setVerticalSyncEnabled(bool enable) const noexcept;
    void setClearColor(const vec4s& clearColor) noexcept;

    void pollEvents() const noexcept;
    void clear() const noexcept;
    void display() const noexcept;

    float  getElapsedTime() const noexcept;
    ivec2s getSize() const noexcept;

    struct GLFWwindow* getGLFWHandle() const noexcept;

    bool isKeyPressed(int key) const noexcept;
    bool isOpen() const noexcept;

private:
    bool createGLFWWindow(const char* title, int width, int height) noexcept;
    void initializeDebug() noexcept;
    void initCallbacks() noexcept;

    struct WindowData*          m_windowData;
    struct GLFWwindow*          m_glfwWindow;
    std::unique_ptr<class View> m_view;

    vec4s m_clearColor;
};