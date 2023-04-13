#pragma once

#include <cstdint>

#include <windows.h>

namespace memory_management
{
    const uintptr_t get_module_address(DWORD process_id, const wchar_t* module_name);

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
}