#include <array>

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <cglm/struct/cam.h>
#include "cglm/struct/affine-mat.h"

#include "files/Shader.hpp"
#include "files/TileMap.hpp"
#include "graphics/texture/Texture2D.hpp"
#include "graphics/program/ShaderProgram.hpp"
#include "graphics/sprites/SpriteManager.hpp"
#include "graphics/camera/View.hpp"
#include "graphics/level/Level.hpp"
#include "game/Game.hpp"


Game::Game() noexcept:
	Window()
{

}

Game::~Game() = default;


bool Game::setup() noexcept
{
	int width = 800;
    int height = 600;
    const char* title = "Platformer 2D";

	if (!openWindow(title, width, height))
		return false;

    if (!initOpenGL())
        return false;

	glfwSetWindowUserPointer(m_glfwWindow, this);

    glfwSetFramebufferSizeCallback(m_glfwWindow, [](GLFWwindow* window, int width, int height)
	{
		glViewport(0, 0, width, height);

		if (auto game = static_cast<Game*>(glfwGetWindowUserPointer(window)); game)
			game->resize(width, height);
	});

	glfwSetKeyCallback(m_glfwWindow, [](GLFWwindow* window, int key, int scancode, int action, int mods)
	{
		if(key == GLFW_KEY_ESCAPE && action == GLFW_PRESS)
        	glfwSetWindowShouldClose(window, GLFW_TRUE);
	});

	return true;
}


int Game::play() noexcept
{
    TileMap tilemap;

    if (!tilemap.loadFromFile("res/levels/Level-1.tmx"))
        return 1;

    Level level;

    if (!level.load(tilemap))
        return 1;

    ShaderProgram program;
    {
        std::array<Shader, 2> shaders;

        if (!shaders[0].loadFromFile("res/shaders/sprite.vert", GL_VERTEX_SHADER)) return 1;
        if (!shaders[1].loadFromFile("res/shaders/sprite.frag", GL_FRAGMENT_SHADER)) return 1;
        if (!program.link(shaders)) return 1;
    }
    glUseProgram(program.getHandle());

    float deltaTime = 0.f;
	float lastFrame = 0.f;

    glClearColor(0.f, 0.f, 0.f, 1.f);

    while (!glfwWindowShouldClose(m_glfwWindow))
	{
        glClear(GL_COLOR_BUFFER_BIT);

		glfwPollEvents();

		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

        if (glfwGetKey(m_glfwWindow, GLFW_KEY_W) == GLFW_PRESS)
        {
            level.move({0, 5});
        }

        if (glfwGetKey(m_glfwWindow, GLFW_KEY_S) == GLFW_PRESS)
        {
            level.move({0, -5});
        }

        if (glfwGetKey(m_glfwWindow, GLFW_KEY_D) == GLFW_PRESS)
        {
            level.move({-5, 0});
        }

        if (glfwGetKey(m_glfwWindow, GLFW_KEY_A) == GLFW_PRESS)
        {
            level.move({5, 0});
        }

        auto projection = getProjectionMatrix();
        auto modelView = level.getMatrix();
        auto mvp = glms_mul(projection, modelView);
        updateUniformBuffer(mvp.raw);

        level.draw();

		glfwSwapBuffers(m_glfwWindow);
	}

	return 0;
}


