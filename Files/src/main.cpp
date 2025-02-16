#include <app.h>

int main()
{
    Application app;
    app.window.title = "Files";
    app.window.size = { 1280, 720 };
    run_application(&app);
    return 0;
}
