#include "Console.h"

#include <iostream>
#include <string>

#include <windows.h>

namespace console
{
    void set_cursor_position(int x, int y)
    {
        static const HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
        std::cout.flush();
        COORD coord = { (SHORT)x, (SHORT)y };
        SetConsoleCursorPosition(console_handle, coord);
    };

    void clear_console(int start_x, int start_y)
    {
        set_cursor_position(start_x, start_y);
        for (size_t i = 0; i < 10; i++)
            std::cout << std::string(60, ' ') << "\n";
    }

    void set_console_color(Color color)
    {
        static const HANDLE console_handle = GetStdHandle(STD_OUTPUT_HANDLE);
        SetConsoleTextAttribute(console_handle, color);
    }
}