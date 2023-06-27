#pragma once

#include <cstdint>
#include <string_view>
#include <vector>

#include <windows.h>

namespace memory_management
{
    struct module_t
    {
        uintptr_t base_address{};
        uintptr_t size{};
    };

    const module_t get_module_data(DWORD process_id, const wchar_t* module_name);

    template<typename T>
    T read_vm(HANDLE process_handle, uintptr_t address)
    {
        T buffer{};
        ReadProcessMemory(process_handle, reinterpret_cast<LPCVOID>(address), &buffer, sizeof(T), 0);
        return buffer;
    }

    template<typename T>
    bool write_vm(HANDLE process_handle, uintptr_t address, T value)
    {
        return WriteProcessMemory(process_handle, reinterpret_cast<LPVOID>(address), &value, sizeof(T), 0);
    }

    uintptr_t find_pattern(const HANDLE process_handle, const std::wstring_view module_name, const std::string_view pattern);

    uintptr_t find_with_multiple_patterns(const HANDLE process_handle, const std::wstring_view module_name, std::vector<std::string_view> patterns);
}