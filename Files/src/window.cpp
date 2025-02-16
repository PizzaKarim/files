#include <window.h>

#include <glad/glad.h>
#include <GLFW/glfw3.h>

#include <shape.h>
#include <text.h>
#include <scene.h>
#include <explorer.h>

void init_window(Window* window)
{
    if (!glfwInit())
    {
        fprintf(stderr, "Failed to initialize GLFW\n");
        exit(1);
        return;
    }

    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
    glfwWindowHint(GLFW_TRANSPARENT_FRAMEBUFFER, GLFW_TRUE);

    // Center window on primary monitor
    const GLFWvidmode* vidmode = glfwGetVideoMode(glfwGetPrimaryMonitor());
    glfwWindowHint(GLFW_POSITION_X, (int)((vidmode->width - window->size.x) >> 1));
    glfwWindowHint(GLFW_POSITION_Y, (int)((vidmode->height - window->size.y) >> 1));

    window->ptr = glfwCreateWindow(window->size.x, window->size.y, window->title.c_str(), nullptr, nullptr);
    if (!window->ptr)
    {
        fprintf(stderr, "Failed to create GLFW window\n");
        glfwTerminate();
        exit(1);
        return;
    }

    glfwMakeContextCurrent((GLFWwindow*)window->ptr);
    if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
    {
        fprintf(stderr, "Failed to initialize GLAD\n");
        glfwTerminate();
        exit(1);
        return;
    }

    // Store pointer to our window instance for use in window callbacks
    glfwSetWindowUserPointer((GLFWwindow*)window->ptr, window);

    glfwSetFramebufferSizeCallback((GLFWwindow*)window->ptr, [](GLFWwindow* handle, int width, int height)
        {
            glViewport(0, 0, width, height);
            Window* window = (Window*)glfwGetWindowUserPointer(handle);
            window->size = { width, height };
            resize_quad(window->size);
            resize_font_projection();
            explorer_resize((float)width, (float)height);
            render_scene(window->time.delta);
            update_window(window);
        });
    glfwSetScrollCallback((GLFWwindow*)window->ptr, [](GLFWwindow* window, double xoffset, double yoffset)
        {
            explorer_scroll((float)xoffset, (float)yoffset);
        });
    glfwSetCursorPosCallback((GLFWwindow*)window->ptr, [](GLFWwindow* window, double xpos, double ypos)
        {
            explorer_cursor((float)xpos, (float)ypos);
        });
    glfwSetMouseButtonCallback((GLFWwindow*)window->ptr, [](GLFWwindow* window, int button, int action, int mods)
        {
            explorer_click(button, action, mods);
        });
    glfwSetKeyCallback((GLFWwindow*)window->ptr, [](GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            explorer_key(key, scancode, action, mods);
            scene_key(key, scancode, action, mods);
        });
    glfwSetCursorEnterCallback((GLFWwindow*)window->ptr, [](GLFWwindow* window, int entered)
        {
            explorer_enter(entered);
        });
    glfwSetCharCallback((GLFWwindow*)window->ptr, [](GLFWwindow* window, unsigned int codepoint)
        {
            scene_input(codepoint);
        });
}

void update_window(const Window* window)
{
    glfwSwapBuffers((GLFWwindow*)window->ptr);
    glfwPollEvents();
}

void destroy_window(const Window* window)
{
    glfwDestroyWindow((GLFWwindow*)window->ptr);
    glfwTerminate();
}

bool running_window(const Window* window)
{
    return !glfwWindowShouldClose((GLFWwindow*)window->ptr);
}

float window_time()
{
    return (float)glfwGetTime();
}
