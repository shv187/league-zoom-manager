#include "Config.h"

#include <windows.h>

void Config::handle_input()
{
    if (GetAsyncKeyState(VK_LEFT) & 0x1)
        camera_height -= zoom_step;

    if (GetAsyncKeyState(VK_RIGHT) & 0x1)
        camera_height += zoom_step;

    if (GetAsyncKeyState(VK_DOWN) & 0x1)
        camera_height = standard_max_zoom;

    if (GetAsyncKeyState(VK_UP) & 0x1)
        camera_height = default_extended_zoom;
}