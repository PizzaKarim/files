#include <app.h>

#include <shape.h>
#include <scene.h>
#include <navigation.h>

#include <glad/glad.h>

#include <stb_image.h>

#include <filesystem>

Application* APP = nullptr;

void run_application(Application* app)
{
    APP = app;
    init_window(&app->window);
    init_quad();
    init_font_renderer();

    app->font = load_font("C:/Windows/Fonts/consola.ttf", 24);

    stbi_set_flip_vertically_on_load(true);
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);

    std::string start_dir = std::filesystem::current_path().string();
    start_dir = "";
    navigate_directory(start_dir);

    app->window.time.last = window_time();
    while (running_window(&app->window))
    {
        app->window.time.current = window_time();
        app->window.time.delta = app->window.time.current - app->window.time.last;
        app->window.time.last = app->window.time.current;

        render_scene(app->window.time.delta);
        update_window(&app->window);
    }
    destroy_quad();
    destroy_font_renderer();
    destroy_window(&app->window);
    APP = nullptr;
}
