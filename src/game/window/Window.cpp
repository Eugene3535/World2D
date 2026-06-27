#include <GLFW/glfw3.h>

#include "game/window/Window.hpp"


Window::Window() noexcept:
    m_window(nullptr)
{

}


Window::~Window()
{
    glfwDestroyWindow(m_window);
	glfwTerminate();
}


bool Window::openWindow(const char* title, int width, int height) noexcept
{
    if (glfwInit() != GLFW_TRUE)
        return false;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    if (m_window = glfwCreateWindow(width, height, title, nullptr, nullptr); !m_window)
        return false;
    
    glfwMakeContextCurrent(m_window);
    glfwSwapInterval(1);

    return true;
}