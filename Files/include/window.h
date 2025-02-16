#pragma once

#include <glm/glm.hpp>
#include <string>

struct FrameTime
{
    float current;
    float last;
    float delta;
};

struct Window
{
    void* ptr; // pointer to GLFW window
    glm::uvec2 size;
    std::string title;
    FrameTime time;
};

void init_window(Window* window);
void update_window(const Window* window);
void destroy_window(const Window* window);

bool running_window(const Window* window);
float window_time();
