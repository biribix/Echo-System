#include "echo/Application.h"

int main()
{
    Echo::Application app(1280, 720, "Echo Engine - Editor");
    app.Run();
    return 0;
}
