#include "Config.h"
#include "League.h"
#include "Console.h"
#include "exceptions.h"

#include <iostream>
#include <thread>
#include <chrono>

#include "xorstr.hpp"

int main()
{
    SetConsoleTitle(xorstr_(L"League Zoom Manager by shv187"));
    console::set_console_color(console::Color::blue);

    std::cout << 
        xorstr_("Welcome to shv187's League Zoom Manager\n"
        "\n"
        "Use <- and -> to adjust zoom with step of 50u.\n"
        "Use ^ to set zoom to default extended value.\n"
        "Use v to set zoom to standard league value.\n"
        "\n");

    try
    {
        while (true)
        {
            League league{};

            if (!league.initialized())
            {
                console::clear_console(0, 6);
                console::set_cursor_position(0, 6);
                std::cout << xorstr_("Waiting for league process.\n");
                std::this_thread::sleep_for(std::chrono::milliseconds(100));
                continue;
            }

            console::clear_console(0, 6);
            console::set_cursor_position(0, 6);
            std::cout << xorstr_("League process has been found!\n\n");
            MessageBeep(MB_OK);

            league.print_informations();

            Config config{ league.get_camera_height(), league.get_camera_fov() };

            while (league.is_process_alive())
            {
                if (!league.is_window_active())
                    continue;

                config.handle_input();

                if (league.get_camera_height() != config.camera_height)
                    league.set_camera_height(config.camera_height);

                std::this_thread::sleep_for(std::chrono::milliseconds(1));
            }
        }
    }
    catch (const std::runtime_error& error)
    {
        MessageBoxA(nullptr, error.what(), xorstr_("Error"), MB_OK | MB_ICONWARNING);
    }
}