#pragma once

#include <cstdint>

#include <windows.h>

class League
{
private:
    enum RVA : uintptr_t
    {
        hud_instance = 0x210DDD8
    };

    enum HudInstance : uintptr_t
    {
        is_background_window = 0xB9,
        camera = 0x18
    };

    enum Camera : uintptr_t
    {
        fov = 0x1B4,
        height = 0x2BC
    };

public:
    League();

public:
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
    uintptr_t m_hud_instance{};
    uintptr_t m_camera_instance{};
};