#include <explorer.h>

#include <glm/gtc/matrix_transform.hpp>
#include <GLFW/glfw3.h>

#include <app.h>
#include <navigation.h>
#include <shape.h>

#if defined(_WIN32)
#include <Windows.h>
#endif

// TODO: Allow using ../ to access drive selection ("C:/../" = "")

extern Application* APP;
extern std::vector<File> files;

static glm::vec2 file_size = { 240, 48 };
static glm::vec2 thumbnail_size = { 40, 40 };
static glm::vec2 thumbnail_offset = {
    ((unsigned int)(file_size.y - thumbnail_size.x)) >> 1,
    ((unsigned int)(file_size.y - thumbnail_size.y)) >> 1
};
static glm::vec2 text_offset = { 4, 4 };
static glm::vec2 margin = { 16, 16 };
static glm::vec2 space = { 12, 12 };

static std::unordered_map<unsigned int, glm::vec2> positions;
static std::vector<unsigned int> selected;
static unsigned int hovered = (unsigned int)-1;

static glm::vec2 constraints;
static glm::uvec2 grid;
static glm::mat4 view;
static float height;
static glm::vec2 mb3_anchor;
static bool mb3_scrolling;
static glm::vec2 offset_size;

static glm::uvec2 calc_grid(glm::vec2 size)
{
    unsigned int x = (unsigned int)glm::floor((size.x - margin.x * 2 + space.x) / (file_size.x + space.x));
    unsigned int y = (unsigned int)glm::floor((size.y - margin.y * 2 + space.y) / (file_size.y + space.y));
    return { max(1u, x), y };
}

static void resize(glm::vec2 size, glm::vec2 offset)
{
    offset_size = offset;
    constraints = size + offset;
    grid = calc_grid(constraints);
}

static void render_file(const File& file, unsigned int index, glm::vec2 position)
{
    glm::mat4 thumbnail = glm::mat4(1.0f);
    thumbnail = glm::translate(thumbnail, glm::vec3(position + thumbnail_offset, 0));
    thumbnail = glm::scale(thumbnail, glm::vec3(thumbnail_size, 0));

    bool s = std::find(selected.begin(), selected.end(), index) != selected.end();
    if (hovered == index || s)
    {
        glm::mat4 highlight = glm::mat4(1.0f);
        highlight = glm::translate(highlight, glm::vec3(position, 0));
        highlight = glm::scale(highlight, glm::vec3(file_size, 0));

        static float hovered_opacity = 0.2f;
        static float selected_opacity = 0.3f;
        float opacity = s ? selected_opacity : hovered_opacity;
        render_quad(view * highlight, 0, { 1.0f, 1.0f, 1.0f, opacity });
    }
    render_quad(view * thumbnail, file.thumbnail, { 1.0f, 1.0f, 1.0f, 1.0f });

    static float max_length = file_size.x - thumbnail_size.x - thumbnail_offset.x - text_offset.x;
    Text text = preview_text(&APP->font, file.name, 0.6f, max_length, "...");
    glm::vec2 text_position = {
        thumbnail_size.x + thumbnail_offset.x * 2 + text_offset.x,
        file_size.y - text.size.y - text_offset.y
    };
    render_text(&text, position + text_position, { 1.0f, 1.0f, 1.0f }, view);
}

static void render_view(glm::vec2 position)
{
    if (mb3_scrolling)
    {
        double xpos, ypos;
        glfwGetCursorPos((GLFWwindow*)APP->window.ptr, &xpos, &ypos);
        glm::vec2 clicked = { xpos, APP->window.size.y - ypos - view[3][1] };

        float dx = (float)xpos - mb3_anchor.x;
        float dy = (float)ypos - mb3_anchor.y;
        if (abs(dy) > 5)
        {
            float y = view[3][1] + (dy > 0 ? dy - 5 : dy + 5) * 0.1f;
            if (y < 0) y = 0;
            unsigned int rows = (unsigned int)glm::ceil((files.size() - 1) / grid.x);
            float height = margin.y * 2 + space.y * rows + file_size.y * (rows + 1);
            float max = height - constraints.y;
            if (y > max) y = max > 0 ? max : 0;
            view[3][1] = y;
        }
    }

    float first_row = (view[3][1] + space.y) / (space.y + file_size.y);
    unsigned int begin = (unsigned int)glm::floor(first_row < 0 ? 0 : first_row) * grid.x;
    if (begin >= grid.x) begin -= grid.x;
    if (begin > files.size() - 1) return;

    positions.clear();
    unsigned int count = grid.x * grid.y + grid.x * 3;
    unsigned int end = min(begin + count, (unsigned int)files.size());
    unsigned int row = 0, column = 0;
    for (unsigned int i = begin; i < end; i++)
    {
        File& file = files[i];
        float x = margin.x + space.x * column + file_size.x * column;
        float width = x + file_size.x;
        if (width > constraints.x - margin.x)
        {
            x = margin.x;
            width = x + file_size.x;
            column = 0;
        }
        column++;
        row = (unsigned int)glm::floor((float)i / grid.x);
        height = margin.y + space.y * row + file_size.y * (row + 1);
        positions.insert({ i, position + glm::vec2(x, constraints.y - height) });
        render_file(file, i, position + glm::vec2(x, constraints.y - height));
    }
}

void render_explorer(glm::vec2 position, glm::vec2 offset_size)
{
    static bool initialized = false;
    if (!initialized)
    {
        //resize({ 1028, 576 });
        resize(APP->window.size, offset_size);
        view = glm::mat4(1.0f);
        initialized = true;
    }
    //glm::mat4 m;
    //glm::vec4 background_color = { 0.0f, 0.0f, 0.0f, 1.0f };

    // Background
    /*
    m = glm::mat4(1.0f);
    m = glm::scale(m, glm::vec3(1280.0f, 720.0f, 0.0f));
    render_quad(m, 0, background_color);
    */

    // Explorer view
    //render_view({ 126, 72 });
    render_view(position);

    /*
    // Border - BOTTOM
    m = glm::mat4(1.0f);
    m = glm::scale(m, glm::vec3(1280.0f, 72.0f, 0.0f));
    render_quad(m, 0, background_color);

    // Border - TOP
    m = glm::mat4(1.0f);
    m = glm::translate(m, glm::vec3(0.0f, 648.0f, 0.0f));
    m = glm::scale(m, glm::vec3(1280.0f, 72.0f, 0.0f));
    render_quad(m, 0, background_color);

    // Border - LEFT
    m = glm::mat4(1.0f);
    m = glm::scale(m, glm::vec3(126.0f, 720.0f, 0.0f));
    render_quad(m, 0, background_color);

    // Border - RIGHT
    m = glm::mat4(1.0f);
    m = glm::translate(m, glm::vec3(1154.0f, 0.0f, 0.0f));
    m = glm::scale(m, glm::vec3(126.0f, 720.0f, 0.0f));
    render_quad(m, 0, background_color);
    */
}

static bool within(glm::vec2 point, glm::vec2 min, glm::vec2 max)
{
    if (point.x < min.x || point.x > max.x) return false;
    if (point.y < min.y || point.y > max.y) return false;
    return true;
}

void explorer_cursor(float x, float y)
{
    if (mb3_scrolling) return;
    y = APP->window.size.y - y - view[3][1];
    hovered = (unsigned int)-1;
    for (const auto& [ index, position ] : positions)
    {
        if (!within({ x, y }, position, position + file_size)) continue;
        hovered = index;
        break;
    }
}

void explorer_enter(int entered)
{
    if (entered) return;
    hovered = (unsigned int)-1;
}

void explorer_scroll(float dx, float dy)
{
    float y = view[3][1] - dy * (file_size.y + space.y);
    if (y < 0) y = 0;
    unsigned int rows = (unsigned int)glm::ceil((files.size() - 1) / grid.x);
    float height = margin.y * 2 + space.y * rows + file_size.y * (rows + 1);
    float max = height - constraints.y;
    if (y > max) y = max > 0 ? max : 0;
    view[3][1] = y;
    double xpos, ypos;
    glfwGetCursorPos((GLFWwindow*)APP->window.ptr, &xpos, &ypos);
    explorer_cursor((float)xpos, (float)ypos);
}

void explorer_resize(float width, float height)
{
    unsigned int rows = (unsigned int)glm::ceil((files.size() - 1) / grid.x);
    float h = margin.y * 2 + space.y * rows + file_size.y * (rows + 1);
    float max = h - constraints.y;
    //if (view[3][1] > max) view[3][1] = max;
    resize({ width, height }, offset_size);
}

static void reset()
{
    view[3][1] = 0;
    hovered = (unsigned int)-1;
    selected.clear();
}

static void open_file(const std::string& file)
{
    if (std::filesystem::is_directory(file))
    {
        if (!navigate_directory(file)) return;
        reset();
        return;
    }
    if (std::filesystem::is_regular_file(file))
    {
#if defined(_WIN32)
        ShellExecuteA(0, 0, file.c_str(), 0, 0, SW_SHOW);
#endif
        return;
    }
}

static void toggle_mb3()
{
    static GLFWcursor* normal = glfwCreateStandardCursor(GLFW_ARROW_CURSOR);
    static GLFWcursor* resize = glfwCreateStandardCursor(GLFW_RESIZE_NS_CURSOR);

    mb3_scrolling = !mb3_scrolling;
    glfwSetCursor((GLFWwindow*)APP->window.ptr, mb3_scrolling ? resize : normal);
    if (mb3_scrolling)
    {
        double xpos, ypos;
        glfwGetCursorPos((GLFWwindow*)APP->window.ptr, &xpos, &ypos);
        //glm::vec2 clicked = { xpos, APP->window.size.y - ypos - view[3][1] };

        mb3_anchor = { xpos, ypos };
        hovered = (unsigned int)-1;
    }
}

void explorer_click(int button, int action, int mods)
{
    if (action != GLFW_PRESS) return;
    if (button == GLFW_MOUSE_BUTTON_3)
    {
        toggle_mb3();
        return;
    }
    if (button == GLFW_MOUSE_BUTTON_4)
    {
        navigate_back();
        reset();
        return;
    }
    if (button == GLFW_MOUSE_BUTTON_5)
    {
        navigate_forward();
        reset();
        return;
    }
    if (button != GLFW_MOUSE_BUTTON_1) return;
    if (mb3_scrolling)
    {
        toggle_mb3();
        return;
    }
    double xpos, ypos;
    glfwGetCursorPos((GLFWwindow*)APP->window.ptr, &xpos, &ypos);
    glm::vec2 clicked = { xpos, APP->window.size.y - ypos - view[3][1] };
    unsigned int pressed_index = (unsigned int)-1;
    for (const auto& [ index, position ] : positions)
    {
        if (within(clicked, position, position + file_size))
        {
            pressed_index = index;
            break;
        }
    }
    if (pressed_index == -1)
    {
        selected.clear();
        return;
    }
    bool pressed = std::find(selected.begin(), selected.end(), pressed_index) != selected.end();
#if defined(_WIN32)
    static float threshold = (float)GetDoubleClickTime() / 1000;
#else
    static float threshold = 0.5f;
#endif
    static float last_click = 0;
    float time = window_time();
    bool double_click = time - last_click < threshold;
    last_click = time;
    if (pressed && double_click)
    {
        open_file(files[pressed_index].path);
        return;
    }
    selected.clear();
    selected.push_back(pressed_index);
}

void explorer_key(int key, int scancode, int action, int mods)
{
    if (action == GLFW_RELEASE) return;
    if (selected.size() == 1 && key == GLFW_KEY_ENTER && action == GLFW_PRESS)
    {
        open_file(files[selected[0]].path);
        return;
    }
    switch (key)
    {
    case GLFW_KEY_UP:
        if (mods & GLFW_MOD_ALT)
        {
            if (!navigate_up()) return;
            reset();
            return;
        }
        if (selected.empty())
        {
            selected.push_back(0);
            view[3][1] = 0;
        }
        if (selected[0] >= grid.x) selected[0] -= grid.x;
        if (constraints.y - positions[selected[0]].y < view[3][1] + (file_size.y + space.y))
        {
            explorer_scroll(0, 1);
        }
        break;
    case GLFW_KEY_DOWN:
        if (mods & GLFW_MOD_ALT) return;
        if (selected.empty())
        {
            selected.push_back(0);
            view[3][1] = 0;
        }
        if (selected[0] + grid.x < files.size()) selected[0] += grid.x;
        if (constraints.y - positions[selected[0]].y > constraints.y + view[3][1])
        {
            explorer_scroll(0, -1);
        }
        break;
    case GLFW_KEY_LEFT:
        if (mods & GLFW_MOD_ALT)
        {
            if (!navigate_back()) return;
            reset();
            return;
        }
        if (selected.empty())
        {
            selected.push_back(0);
            view[3][1] = 0;
        }
        if (selected[0] > 0) selected[0] -= 1;
        if (constraints.y - positions[selected[0]].y < view[3][1] + (file_size.y + space.y))
        {
            explorer_scroll(0, 1);
        }
        break;
    case GLFW_KEY_RIGHT:
        if (mods & GLFW_MOD_ALT)
        {
            if (!navigate_forward()) return;
            reset();
            return;
        }
        if (selected.empty())
        {
            selected.push_back(0);
            view[3][1] = 0;
        }
        if (selected[0] < files.size() - 1) selected[0] += 1;
        if (constraints.y - positions[selected[0]].y > constraints.y + view[3][1])
        {
            explorer_scroll(0, -1);
        }
        break;
    default:
        return;
    }
}
