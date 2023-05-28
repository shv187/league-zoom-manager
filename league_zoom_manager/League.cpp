#include "League.h"

#include "MemoryManagement.h"

#include <iostream>
#include <format>

namespace offsets
{
    bool initialized{ false };

    uintptr_t camera{};
    uintptr_t is_background_window{};
    uintptr_t height{};
}

League::League()
{
    m_window_handle = FindWindow(L"RiotWindowClass", NULL);
    GetWindowThreadProcessId(m_window_handle, &m_process_id);
    m_process_handle = OpenProcess(PROCESS_ALL_ACCESS, NULL, m_process_id);
    m_base_module = memory_management::get_module_address(m_process_id, L"League of Legends.exe");

    std::cout << "Pattern scanning started...\n";

    static const std::vector<std::string_view> hud_patterns
    {
        "48 8B 0D ?? ?? ?? ?? 8B 57 10",
        "48 8B 0D ?? ?? ?? ?? 44 0F B6 FE",
        "48 8B 05 ?? ?? ?? ?? 48 8B 48 28 48 85 FF"
    };
    auto hud_match = memory_management::find_with_multiple_patterns(m_process_handle, "League of Legends.exe", hud_patterns);
    auto hud_offset = memory_management::read_vm<uint32_t>(m_process_handle, hud_match + 3);
    m_hud_instance = memory_management::read_vm<uintptr_t>(m_process_handle, hud_match + hud_offset + 7);


    if (!offsets::initialized)
    {
        const std::vector<std::string_view> camera_patterns
        {
            "48 8B 78 ?? 48 8B CF E8 ?? ?? ?? ?? 49 8B D6",
            "48 8B 78 ?? 0F B6 42 0E",
            "48 8B 48 ?? 45 84 C0"
        };
        const auto camera_match = memory_management::find_with_multiple_patterns(m_process_handle, "League of Legends.exe", camera_patterns);
        offsets::camera = memory_management::read_vm<uint8_t>(m_process_handle, camera_match + 3);

        const std::vector<std::string_view> is_background_window_patterns
        {
            "80 B8 ?? 00 00 00 00 74 22",
            "80 BE ?? 00 00 00 00 0F 85 CE 00 00 00",
            "80 B8 ?? 00 00 00 00 75 0F"
        };
        const auto is_background_window_match = memory_management::find_with_multiple_patterns(m_process_handle, "League of Legends.exe", is_background_window_patterns);
        offsets::is_background_window = memory_management::read_vm<int32_t>(m_process_handle, is_background_window_match + 2);

        const std::vector<std::string_view> height_patterns
        {
            "F3 0F 10 8B ?? ?? ?? ?? 66 0F 6E C0",
            "C1 0F 2E 83 ?? ?? ?? ?? B0 01 74 1A",
        };
        const auto height_match = memory_management::find_with_multiple_patterns(m_process_handle, "League of Legends.exe", height_patterns);
        offsets::height = memory_management::read_vm<uint32_t>(m_process_handle, height_match + 4);

        offsets::initialized = true;
    }

    m_camera_instance = memory_management::read_vm<uintptr_t>(m_process_handle, m_hud_instance + offsets::camera);
}

void League::set_camera_height(float new_height)
{
    memory_management::write_vm<float>(m_process_handle, m_camera_instance + offsets::height, new_height);
}

void League::set_camera_fov(float new_fov)
{
    memory_management::write_vm<float>(m_process_handle, m_camera_instance + Camera::fov, new_fov);
}

float League::get_camera_height() const
{
    return memory_management::read_vm<float>(m_process_handle, m_camera_instance + offsets::height);
}

float League::get_camera_fov() const
{
    return memory_management::read_vm<float>(m_process_handle, m_camera_instance + Camera::fov);
}

bool League::is_window_active() const
{
    return !memory_management::read_vm<bool>(m_process_handle, m_hud_instance + offsets::is_background_window);
}

bool League::initialized() const
{
    return m_window_handle != INVALID_HANDLE_VALUE &&
        m_process_id &&
        m_base_module &&
        m_hud_instance &&
        m_camera_instance &&
        offsets::initialized;
}

bool League::is_process_alive() const
{
    DWORD exit_code{};
    return GetExitCodeProcess(m_process_handle, &exit_code) && exit_code == STILL_ACTIVE;
}

void League::print_informations() const
{
    std::cout << std::format("Process ID: {}\n", m_process_id);
    std::cout << std::format("Base Module Address: 0x{:X}\n", m_base_module);
    std::cout << std::format("Hud Instance Address: 0x{:X}\n", m_hud_instance);
    std::cout << std::format("Camera Instance Address: 0x{:X}\n", m_camera_instance);
}