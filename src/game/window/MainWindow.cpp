#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "graphics/camera/View.hpp"
#include "game/context/Context.hpp"
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


bool MainWindow::open(const char* title, int width, int height, View& view) noexcept
{
    if (glfwInit() != GLFW_TRUE)
        return false;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#ifdef DEBUG
    glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif

    if (m_glfwWindow = glfwCreateWindow(width, height, title, nullptr, nullptr); !m_glfwWindow)
        return false;
    
    glfwMakeContextCurrent(m_glfwWindow);
    glfwSwapInterval(1);

    if (!Context().initialize())
        return false;

    setupCallbacks(view);

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


bool MainWindow::isKeyPressed(int key) const noexcept
{
    return (glfwGetKey(m_glfwWindow, key) == GLFW_PRESS);
}


bool MainWindow::isOpen() const noexcept
{
    return (!glfwWindowShouldClose(m_glfwWindow));
}


void MainWindow::setupCallbacks(View& view) noexcept
{
    glfwSetWindowUserPointer(m_glfwWindow, static_cast<void*>(&view));

    glfwSetFramebufferSizeCallback(m_glfwWindow, [](GLFWwindow* window, int width, int height)
	{
		glViewport(0, 0, width, height);

		if (auto* view = static_cast<View*>(glfwGetWindowUserPointer(window)); view)
            view->resize(width, height);
	});

	glfwSetKeyCallback(m_glfwWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        	glfwSetWindowShouldClose(window, GLFW_TRUE);
	});
}