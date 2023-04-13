#pragma once

#include <cstdint>

namespace console
{
    enum Color : uint16_t
    {
        purple = 5,
        pink = 13,
        blue = 9
    };

    void set_cursor_position(int x, int y);
    void clear_console(int start_x, int start_y);
    void set_console_color(Color color);
}