#include <scene.h>

#include <explorer.h>
#include <shape.h>
#include <app.h>
#include <navigation.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glad/glad.h>
#include <GLFW/glfw3.h>

extern Application* APP;

static std::string input;

void render_scene(float delta_time)
{
    glClearColor(0.0f, 0.0f, 0.0f, 0.5f);
    glClear(GL_COLOR_BUFFER_BIT);
    render_explorer({}, glm::vec2(0, -50));

    glm::mat4 m = glm::mat4(1.0f);
    m = glm::translate(m, glm::vec3(0, APP->window.size.y - 50, 0));
    m = glm::scale(m, glm::vec3(APP->window.size.x, 50, 0));
    render_quad(m, 0, { 0.5f, 0.0f, 0.0f, 1.0f });
    
    Text text = preview_text(&APP->font, input, 0.6f);
    render_text(&text, glm::vec2(20, APP->window.size.y - 20), {1.0f, 1.0f, 1.0f}, glm::mat4(1.0f));

    static float caret_time = 0;
    static float caret_threshold = 0.5f;
    if ((caret_time += delta_time) >= caret_threshold)
    {
        glm::mat4 caret = glm::mat4(1.0f);
        caret = glm::translate(caret, glm::vec3(text.size.x + 22, APP->window.size.y - 20, 0));
        caret = glm::scale(caret, glm::vec3(1, 14, 0));
        render_quad(caret, 0, { 1.0f, 1.0f, 1.0f, 1.0f });
        if (caret_time >= caret_threshold * 2) caret_time -= caret_threshold * 2;
    }
}

static void open_dialog(const std::string& message)
{
    // TODO: Implement GUI and lock input
    printf("%s\n", message.c_str());
}

static void enter_path()
{
    if (input.empty()) return; // TODO: When searchbar or file selection is done; navigate to root when empty
    if (std::filesystem::is_directory(input))
    {
        navigate_directory(input);
        input = "";
    }
    else if (std::filesystem::is_regular_file(input))
    {
        printf("Open file: %s\n", input.c_str());
        input = "";
    }
    else
    {
        printf("Invalid path: %s\n", input.c_str());
    }
}

void scene_key(int key, int scancode, int action, int mods)
{
    if (action == GLFW_RELEASE) return;
    switch (key)
    {
    case GLFW_KEY_BACKSPACE:
        if (!input.empty()) input.pop_back();
        break;
    case GLFW_KEY_ENTER:
        // TODO: Check if the selection is on the searchbar or a file (indicate searchbar with caret)
        enter_path();
        break;
    default:
        break;
    }
}

void scene_input(unsigned int codepoint)
{
    input += (char)codepoint;
}
