#include "League.h"

#include "MemoryManagement.h"
#include "exceptions.h"

#include <iostream>
#include <format>

#include "xorstr.hpp"

namespace offsets
{
    bool initialized{ false };

    uintptr_t client{};
    uintptr_t hud{};
    uintptr_t camera{};
    uintptr_t is_background_window{};
    uintptr_t height{};
}

namespace patterns
{
    const std::vector<std::string_view> client
    {
        ("48 8B 05 ? ? ? ? 8B 58 0C"),
        // TODO: add more sigs
    };

    const std::vector<std::string_view> hud_instance
    {
        ("48 8B 0D ?? ?? ?? ?? 8B 57 10"),
        ("48 8B 0D ?? ?? ?? ?? 44 0F B6 FE"),
        ("48 8B 05 ?? ?? ?? ?? 48 8B 48 28 48 85 FF")
    };

    const std::vector<std::string_view> camera_instance
    {
        ("48 8B 78 ?? 48 8B CF E8 ?? ?? ?? ?? 49 8B D6"),
        ("48 8B 78 ?? 0F B6 42 0E"),
        ("48 8B 48 ?? 45 84 C0")
    };

    const std::vector<std::string_view> is_background_window
    {
        ("80 B8 ?? 00 00 00 00 74 22"),
        ("80 BE ?? 00 00 00 00 0F 85 CE 00 00 00"),
        ("80 B8 ?? 00 00 00 00 75 0F")
    };

    const std::vector<std::string_view> camera_height
    {
        ("F3 0F 10 8B ?? ?? ?? ?? 66 0F 6E C0"),
        ("C1 0F 2E 83 ?? ?? ?? ?? B0 01 74 1A")
    };
}

League::League()
{
    m_window_handle = FindWindow(L"RiotWindowClass", NULL);
    if (m_window_handle == INVALID_HANDLE_VALUE)
        return;

    GetWindowThreadProcessId(m_window_handle, &m_process_id);
    if (!m_process_id)
        return;

    m_process_handle = OpenProcess(PROCESS_ALL_ACCESS, NULL, m_process_id);
    if (m_process_handle == INVALID_HANDLE_VALUE)
        THROW_EXCEPTION(xorstr_("Failed while opening handle to League's process"));

    auto module_data = memory_management::get_module_data(m_process_id, L"League of Legends.exe");

    m_base_module = module_data.base_address;
    if (!m_base_module)
        THROW_EXCEPTION(xorstr_("Failed while getting 'League of Legends.exe' base module addres"));

    if (!offsets::initialized)
    {
        const auto client_match = memory_management::find_with_multiple_patterns(m_process_handle, L"League of Legends.exe", patterns::client);
        const auto client_relative_offset = memory_management::read_vm<uint32_t>(m_process_handle, client_match + 3);
        offsets::client = (client_match + client_relative_offset + 7) - m_base_module;

        const auto hud_match = memory_management::find_with_multiple_patterns(m_process_handle, L"League of Legends.exe", patterns::hud_instance);
        const auto hud_relative_offset = memory_management::read_vm<uint32_t>(m_process_handle, hud_match + 3);
        offsets::hud = (hud_match + hud_relative_offset + 7) - m_base_module;
        if (!offsets::hud)
            THROW_EXCEPTION(xorstr_("Failed while getting hud_instance offset"));

        const auto camera_match = memory_management::find_with_multiple_patterns(m_process_handle, L"League of Legends.exe", patterns::camera_instance);
        offsets::camera = memory_management::read_vm<uint8_t>(m_process_handle, camera_match + 3);
        if (!offsets::camera)
            THROW_EXCEPTION(xorstr_("Failed while getting camera_instance offset"));
        
        const auto is_background_window_match = memory_management::find_with_multiple_patterns(m_process_handle, L"League of Legends.exe", patterns::is_background_window);
        offsets::is_background_window = memory_management::read_vm<int32_t>(m_process_handle, is_background_window_match + 2);
        if (!offsets::is_background_window)
            THROW_EXCEPTION(xorstr_("Failed while getting is_background_window offset"));
        
        const auto height_match = memory_management::find_with_multiple_patterns(m_process_handle, L"League of Legends.exe", patterns::camera_height);
        offsets::height = memory_management::read_vm<uint32_t>(m_process_handle, height_match + 4);
        if (!offsets::height)
            THROW_EXCEPTION(xorstr_("Failed while getting camera_height offset"));

        offsets::initialized = true;
    }

    m_client_instance = memory_management::read_vm<uintptr_t>(m_process_handle, m_base_module + offsets::client);
    if (!m_client_instance)
        THROW_EXCEPTION(xorstr_("Failed while getting client_instance"));

    if (get_game_state() != game_state::running)
        return;

    m_hud_instance = memory_management::read_vm<uintptr_t>(m_process_handle, m_base_module + offsets::hud);
    if (!m_hud_instance)
        THROW_EXCEPTION(xorstr_("Failed while getting hud_instance"));

    m_camera_instance = memory_management::read_vm<uintptr_t>(m_process_handle, m_hud_instance + offsets::camera);
    if (!m_camera_instance)
        THROW_EXCEPTION(xorstr_("Failed while getting camera_instance"));

    m_initialized = true;
}

League::game_state League::get_game_state()
{
    return memory_management::read_vm<League::game_state>(m_process_handle, m_client_instance + Client::state);
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
    return m_initialized;
}

bool League::is_process_alive() const
{
    DWORD exit_code{};
    return GetExitCodeProcess(m_process_handle, &exit_code) && exit_code == STILL_ACTIVE;
}

void League::print_informations() const
{
    std::cout << std::format("[+] Process ID: {}\n", m_process_id);
    std::cout << std::format("[+] Base Module Address: 0x{:X}\n", m_base_module);
    std::cout << std::format("[+] Hud Instance Address: 0x{:X}\n", m_hud_instance);
    std::cout << std::format("[+] Camera Instance Address: 0x{:X}\n", m_camera_instance);
}