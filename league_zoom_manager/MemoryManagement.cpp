#include "MemoryManagement.h"

#include <windows.h>
#include <tlhelp32.h> 

namespace memory_management
{
    const uintptr_t get_module_address(DWORD process_id, const wchar_t* module_name)
    {
        HANDLE snapshot_handle = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, process_id);

        if (snapshot_handle == INVALID_HANDLE_VALUE)
            return 0;

        uintptr_t address{};
        MODULEENTRY32 module_entry{};
        module_entry.dwSize = sizeof(module_entry);

        if (Module32First(snapshot_handle, &module_entry))
        {
            do
            {
                if (!_wcsicmp(module_entry.szModule, module_name))
                {
                    address = reinterpret_cast<uintptr_t>(module_entry.modBaseAddr);
                    break;
                }
            } while (Module32Next(snapshot_handle, &module_entry));
        }

        CloseHandle(snapshot_handle);

        return address;
    }
}