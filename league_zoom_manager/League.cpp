#include "League.h"

#include "MemoryManagement.h"

#include <iostream>
#include <format>

League::League()
{
    m_window_handle = FindWindow(L"RiotWindowClass", NULL);
    GetWindowThreadProcessId(m_window_handle, &m_process_id);
    m_process_handle = OpenProcess(PROCESS_ALL_ACCESS, NULL, m_process_id);
    m_base_module = memory_management::get_module_address(m_process_id, L"League of Legends.exe");

    m_hud_instance = memory_management::read_vm<uintptr_t>(m_process_handle, m_base_module + RVA::hud_instance);
    m_camera_instance = memory_management::read_vm<uintptr_t>(m_process_handle, m_hud_instance + HudInstance::camera);
}

void League::set_camera_height(float new_height)
{
    memory_management::write_vm<float>(m_process_handle, m_camera_instance + Camera::height, new_height);
}

void League::set_camera_fov(float new_fov)
{
    memory_management::write_vm<float>(m_process_handle, m_camera_instance + Camera::fov, new_fov);
}

float League::get_camera_height() const
{
    return memory_management::read_vm<float>(m_process_handle, m_camera_instance + Camera::height);
}

float League::get_camera_fov() const
{
    return memory_management::read_vm<float>(m_process_handle, m_camera_instance + Camera::fov);
}

bool League::is_window_active() const
{
    return !memory_management::read_vm<bool>(m_process_handle, m_hud_instance + HudInstance::is_background_window);
}

bool League::initialized() const
{
    return m_window_handle != INVALID_HANDLE_VALUE &&
        m_process_id &&
        m_base_module &&
        m_hud_instance &&
        m_camera_instance;
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