#pragma once

#include <cstdint>

#include <windows.h>

class League
{
private:
    enum Camera : uintptr_t
    {
        // left it here cuz i cba searching for unique patterns
        // as its not used anyways
        fov = 0x1B4
    };

    enum Client : uintptr_t
    {
        state = 0xC
    };

public:
    League();

public:
    // ty
    // https://github.com/R3nzTheCodeGOD/R3nzSkin/blob/eba74d3b5b056d6f31681a3ecd84ccab2c05db9b/R3nzSkin/SDK/GameState.hpp#L5
    enum game_state : int32_t
    {
        loading_screen = 0,
        connecting = 1,
        running = 2,
        paused = 3,
        finished = 4,
        exiting = 5
    };

    game_state get_game_state();

    void set_camera_height(float new_height);
    void set_camera_fov(float new_fov);

    float get_camera_height() const;
    float get_camera_fov() const;

    bool is_window_active() const;
    bool initialized() const;
    bool is_process_alive() const;

    void print_informations() const;

private:
    HWND m_window_handle{};
    DWORD m_process_id{};
    HANDLE m_process_handle{};
    uintptr_t m_base_module{};

private:
    uintptr_t m_client_instance{};
    uintptr_t m_hud_instance{};
    uintptr_t m_camera_instance{};

private:
    bool m_initialized{ false };
};