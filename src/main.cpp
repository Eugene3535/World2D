#ifdef _WIN32
#include <windows.h>
extern "C"
{
    __declspec(dllexport) unsigned long NvOptimusEnablement = 0x00000001;
    __declspec(dllexport) int AmdPowerXpressRequestHighPerformance = 1;
}
#endif

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include "files/TileMap.hpp"


GLFWwindow* createGLFWwindow() noexcept
{
    int width = 800;
    int height = 600;
    const char* title = "Platformer 2D";
    GLFWwindow* window;

    if (glfwInit() != GLFW_TRUE)
        return nullptr;

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    if (window = glfwCreateWindow(width, height, title, nullptr, nullptr) ; !window)
        return nullptr;
    
    glfwMakeContextCurrent(window);
    glfwSwapInterval(1);

    return window;
}


bool initOpenGL() noexcept
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

#ifdef DEBUG
    printf("All OpenGL types match expected types\n");
#endif
    return true;
}



class Player
{
public:
    Player(float x0, float y0) noexcept
    {
        speed = 5;
        onGround = false;
    }

    void checkCollision(float Dx, float Dy) noexcept
    {

    }

    void update(GLFWwindow* window, float dt) noexcept
    {
        if ((glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS) && onGround)
        {
            onGround = false;
            dy = 100;
        }

        if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
        {

        }

        if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
        {

        }

        if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
        {

        }

        if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
        {

        }
    }

    void draw() noexcept
    {

    }

    float x, y;
    float dx, dy;
    int left, top, width, height; // width, height, depth
    bool onGround;
    float speed;
};



int main()
{
    GLFWwindow* window = createGLFWwindow();

    if (!window)
        return 1;

    if (!initOpenGL())
        return 1;

    TileMap tm;

    if (!tm.loadFromFile("res/levels/Level-1.tmx"))
        return 1;

    float deltaTime = 0.f;
	float lastFrame = 0.f;

    while (!glfwWindowShouldClose(window))
	{
		glfwPollEvents();

		float currentFrame = static_cast<float>(glfwGetTime());
		deltaTime = currentFrame - lastFrame;
		lastFrame = currentFrame;


		glfwSwapBuffers(window);
	}

    glfwDestroyWindow(window);
	glfwTerminate();

	return 0;
}

