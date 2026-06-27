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
	m_window(nullptr)
{

}

Game::~Game()
{
    glfwDestroyWindow(m_window);
	glfwTerminate();
}


bool Game::setupContext() noexcept
{
	int width = 800;
    int height = 600;
    const char* title = "Platformer 2D";
    GLFWwindow* window = openWindow(title, width, height);

    if (!window)
        return false;

	m_window = window;

    if (!initOpenGL())
        return false;

    initDebug();

	m_view = std::make_unique<View>();
	m_view->create(width, height);

	glfwSetWindowUserPointer(m_window, this);

    glfwSetFramebufferSizeCallback(m_window, [](GLFWwindow* window, int width, int height)
	{
		glViewport(0, 0, width, height);

		if (auto game = static_cast<Game*>(glfwGetWindowUserPointer(window)); game)
			game->m_view->resize(width, height);
	});

	glfwSetKeyCallback(m_window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
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

    while (!glfwWindowShouldClose(m_window))
	{
        glClear(GL_COLOR_BUFFER_BIT);

		glfwPollEvents();

		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;

        if (glfwGetKey(m_window, GLFW_KEY_W) == GLFW_PRESS)
        {
            level.move({0, 5});
        }

        if (glfwGetKey(m_window, GLFW_KEY_S) == GLFW_PRESS)
        {
            level.move({0, -5});
        }

        if (glfwGetKey(m_window, GLFW_KEY_D) == GLFW_PRESS)
        {
            level.move({-5, 0});
        }

        if (glfwGetKey(m_window, GLFW_KEY_A) == GLFW_PRESS)
        {
            level.move({5, 0});
        }

        auto projection = m_view->getProjectionMatrix();
        auto modelView = level.getMatrix();
        auto mvp = glms_mul(projection, modelView);
        m_view->updateUniformBuffer(mvp.raw);

        level.draw();

		glfwSwapBuffers(m_window);
	}

	return 0;
}


GLFWwindow* Game::openWindow(const char* title, int width, int height) noexcept
{
    GLFWwindow* window;

    if (glfwInit() != GLFW_TRUE)
        return nullptr;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    if (window = glfwCreateWindow(width, height, title, nullptr, nullptr); !window)
        return nullptr;
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    return window;
}


bool Game::initOpenGL() noexcept
{
    if (!gladLoadGL())
        return false;

    static_assert(std::is_same_v<GLbyte, int8_t>, "GLbyte mismatch");
    static_assert(std::is_same_v<GLubyte, uint8_t>, "GLubyte mismatch");
    static_assert(std::is_same_v<GLshort, int16_t>, "GLshort mismatch");
    static_assert(std::is_same_v<GLushort, uint16_t>, "GLushort mismatch");
    static_assert(std::is_same_v<GLint, int32_t>, "GLint mismatch");
    static_assert(std::is_same_v<GLuint, uint32_t>, "GLuint mismatch");
    static_assert(std::is_same_v<GLfloat, float>, "GLfloat mismatch");
    static_assert(std::is_same_v<GLdouble, double>, "GLdouble mismatch");

    return true;
}


void Game::initDebug() noexcept
{
#ifdef DEBUG
	printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
	printf("GLSL Version: %s\n", glGetString(GL_SHADING_LANGUAGE_VERSION));
	printf("Vendor: %s\n", glGetString(GL_VENDOR));
	printf("Renderer: %s\n", glGetString(GL_RENDERER));

	glEnable(GL_DEBUG_OUTPUT);
	glEnable(GL_DEBUG_OUTPUT_SYNCHRONOUS);

	glDebugMessageCallback([](GLenum source, GLenum type, GLuint id, GLenum severity, GLsizei length, GLchar const* message, void const* user_param)
	{
		auto source_str = [source]() -> const char*
		{
			switch (source)
			{
				case GL_DEBUG_SOURCE_API:             return "API";
				case GL_DEBUG_SOURCE_WINDOW_SYSTEM:   return "WINDOW SYSTEM";
				case GL_DEBUG_SOURCE_SHADER_COMPILER: return "SHADER COMPILER";
				case GL_DEBUG_SOURCE_THIRD_PARTY:     return "THIRD PARTY";
				case GL_DEBUG_SOURCE_APPLICATION:     return "APPLICATION";
				case GL_DEBUG_SOURCE_OTHER:           return "OTHER";
				
				default: return "UNKNOWN";
			}
		}();

		auto type_str = [type]() -> const char*
		{
			switch (type)
			{
				case GL_DEBUG_TYPE_ERROR:               return "ERROR";
				case GL_DEBUG_TYPE_DEPRECATED_BEHAVIOR: return "DEPRECATED_BEHAVIOR";
				case GL_DEBUG_TYPE_UNDEFINED_BEHAVIOR:  return "UNDEFINED_BEHAVIOR";
				case GL_DEBUG_TYPE_PORTABILITY:         return "PORTABILITY";
				case GL_DEBUG_TYPE_PERFORMANCE:         return "PERFORMANCE";
				case GL_DEBUG_TYPE_MARKER:              return "MARKER";
				case GL_DEBUG_TYPE_OTHER:               return "OTHER";

				default: return "UNKNOWN";
			}
		}();

		auto severity_str = [severity]() -> const char*
		{
			switch (severity) 
			{
				case GL_DEBUG_SEVERITY_NOTIFICATION: return "NOTIFICATION";
				case GL_DEBUG_SEVERITY_LOW:          return "LOW";
				case GL_DEBUG_SEVERITY_MEDIUM:       return "MEDIUM";
				case GL_DEBUG_SEVERITY_HIGH:         return "HIGH";

				default: return "UNKNOWN";
			}
		}();

		printf("%s, %s, %s, %u: %s\n", source_str, type_str, severity_str, id, message);
	}, nullptr);
#endif
}