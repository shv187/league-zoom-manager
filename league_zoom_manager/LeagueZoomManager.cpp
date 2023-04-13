#include "LeagueZoomManager.h"

#include "Config.h"
#include "League.h"
#include "Console.h"

#include <iostream>
#include <thread>
#include <chrono>

LeagueZoomManager::LeagueZoomManager()
{
    SetConsoleTitle(L"League Zoom Manager by shv187");
    console::set_console_color(console::Color::blue);

    std::cout << "Welcome to shv187's League Zoom Manager.\n\n";

    std::cout << "Use <- and -> to adjust zoom with step of 50u.\n";
    std::cout << "Use ^ to set zoom to default extended value.\n";
    std::cout << "Use v to set zoom to standard league value.\n\n";
}

void LeagueZoomManager::run()
{
    while (true)
    {
        League league{};

        if (!league.initialized())
        {
            console::clear_console(0, 6);
            console::set_cursor_position(0, 6);
            std::cout << "Waiting for league process.\n";
            std::this_thread::sleep_for(std::chrono::milliseconds(100));
            continue;
        }

        console::clear_console(0, 6);
        console::set_cursor_position(0, 6);
        std::cout << "League process has been found!\n\n";

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