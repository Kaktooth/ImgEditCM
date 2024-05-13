#include "gui.cpp"

using namespace std::chrono;

int WINAPI WinMain(HINSTANCE hInstance, HINSTANCE hPrevInstance,
    PSTR lpCmdLine, int nCmdShow)
{
    Gui gui;
    gui.init(1972, 1240);
    gui.draw();
    gui.clear();
}
