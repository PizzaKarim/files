#pragma once

#include <window.h>
#include <text.h>

struct Application
{
    Window window;
    Font font;
};

void run_application(Application* app);
