#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "graphics/camera/View.hpp"
#include "game/context/Context.hpp"
#include "game/window/WindowData.hpp"
#include "game/window/MainWindow.hpp"



MainWindow::MainWindow() noexcept:
    m_glfwWindow(nullptr)
{

}


MainWindow::~MainWindow()
{
    glfwDestroyWindow(m_glfwWindow);
	glfwTerminate();
}


bool MainWindow::open(WindowData& data) noexcept
{
    if (glfwInit() != GLFW_TRUE)
        return false;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

    if (m_glfwWindow = glfwCreateWindow(data.width, data.height, data.title.c_str(), nullptr, nullptr); !m_glfwWindow)
        return false;
    
    glfwMakeContextCurrent(m_glfwWindow);
    glfwSwapInterval(1);

    if (!Context().initialize())
        return false;

    setupCallbacks(data);

    return true;
}


void MainWindow::close() const noexcept
{
    glfwSetWindowShouldClose(m_glfwWindow, GLFW_TRUE);
}


void MainWindow::pollEvents() const noexcept
{
    glfwPollEvents();
}


void MainWindow::display() const noexcept
{
    glfwSwapBuffers(m_glfwWindow);
}


float MainWindow::getElapsedTime() const noexcept
{
    return static_cast<float>(glfwGetTime());
}


ivec2s MainWindow::getSize() const noexcept
{
    int width, height;
	glfwGetWindowSize(m_glfwWindow, &width, &height);

    return { width, height };
}


bool MainWindow::isKeyPressed(char symbol) const noexcept
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


bool MainWindow::isOpen() const noexcept
{
    return (!glfwWindowShouldClose(m_glfwWindow));
}


void MainWindow::setupCallbacks(WindowData& data) noexcept
{
    glfwSetWindowUserPointer(m_glfwWindow, static_cast<void*>(&data));

    glfwSetFramebufferSizeCallback(m_glfwWindow, [](GLFWwindow* window, int width, int height)
	{
		glViewport(0, 0, width, height);

		if (auto data = static_cast<WindowData*>(glfwGetWindowUserPointer(window)); data)
        {
            if (auto* view = static_cast<View*>(data->view); view)
                view->resize(width, height);

            data->width = width;
            data->height = height;
        }
	});

	glfwSetKeyCallback(m_glfwWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        	glfwSetWindowShouldClose(window, GLFW_TRUE);
	});
}