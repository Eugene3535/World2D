#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "game/window/WindowData.hpp"
#include "game/window/Window.hpp"



Window::Window() noexcept:
    m_glfwWindow(nullptr)
{

}


Window::~Window()
{
    glfwDestroyWindow(m_glfwWindow);
	glfwTerminate();
}


bool Window::open(WindowData* data) noexcept
{
    if (glfwInit() != GLFW_TRUE)
        return false;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

    if (m_glfwWindow = glfwCreateWindow(data->width, data->height, data->title.c_str(), nullptr, nullptr); !m_glfwWindow)
        return false;
    
    glfwMakeContextCurrent(m_glfwWindow);
    glfwSwapInterval(1);

    glfwSetWindowUserPointer(m_glfwWindow, &data);

    glfwSetFramebufferSizeCallback(m_glfwWindow, [](GLFWwindow* window, int width, int height)
	{
		glViewport(0, 0, width, height);

		if (auto data = static_cast<WindowData*>(glfwGetWindowUserPointer(window)); data)
        {
			data->view.resize(width, height);
            data->width = width;
            data->height = height;
        }
	});

	glfwSetKeyCallback(m_glfwWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        	glfwSetWindowShouldClose(window, GLFW_TRUE);
	});

    return true;
}


void Window::close() const noexcept
{
    glfwSetWindowShouldClose(m_glfwWindow, GLFW_TRUE);
}


void Window::pollEvents() const noexcept
{
    glfwPollEvents();
}


void Window::display() const noexcept
{
    glfwSwapBuffers(m_glfwWindow);
}


float Window::getElapsedTime() const noexcept
{
    return static_cast<float>(glfwGetTime());
}


ivec2s Window::getSize() const noexcept
{
    int width, height;
	glfwGetWindowSize(m_glfwWindow, &width, &height);

    return { width, height };
}


bool Window::isKeyPressed(char symbol) const noexcept
{
    auto char_to_glfw_key = [](char keyCode) -> int
    {
        switch (keyCode)
        {
            case ' ': return GLFW_KEY_SPACE;
            case 'A': return GLFW_KEY_A;
            case 'D': return GLFW_KEY_D;
            case 'S': return GLFW_KEY_S;
            case 'W': return GLFW_KEY_W;

            default: return 0;
        }
    };

    const int keyCode = char_to_glfw_key(symbol);

    return keyCode ? (glfwGetKey(m_glfwWindow, keyCode) == GLFW_PRESS) : false;
}


bool Window::isOpen() const noexcept
{
    return (!glfwWindowShouldClose(m_glfwWindow));
}