#include "MemoryManagement.h"

#include <vector>
#include <sstream>

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

    const uintptr_t get_module_size(DWORD process_id, const wchar_t* module_name)
    {
        HANDLE snapshot_handle = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, process_id);

        if (snapshot_handle == INVALID_HANDLE_VALUE)
            return 0;

        uintptr_t size{};
        MODULEENTRY32 module_entry{};
        module_entry.dwSize = sizeof(module_entry);

        if (Module32First(snapshot_handle, &module_entry))
        {
            do
            {
                if (!_wcsicmp(module_entry.szModule, module_name))
                {
                    size = module_entry.modBaseSize;
                    break;
                }
            } while (Module32Next(snapshot_handle, &module_entry));
        }

        CloseHandle(snapshot_handle);

        return size;
    }

    const std::vector<int> pattern_to_bytes(const std::string_view input)
    {
        auto bytes = std::vector<int>{};
        auto stream = std::istringstream(input.data());
        auto buffer = std::string{};

        while (std::getline(stream, buffer, ' '))
        {
            if (buffer == "?" || buffer == "??")
            {
                bytes.push_back(-1);
            }
            else if (buffer.substr(0, 2) == "0x")
            {
                bytes.push_back(std::stoul(buffer.substr(2), nullptr, 16));
            }
            else
            {
                bytes.push_back(std::stoul(buffer, nullptr, 16));
            }
        }

        return bytes;
    }

    const uint32_t get_process_id_from_process_handle(HANDLE handle)
    {
        return GetProcessId(handle);
    }

    uintptr_t find_pattern(const HANDLE process_handle, const std::string_view module_name, const std::string_view pattern)
    {
        const auto process_id = get_process_id_from_process_handle(process_handle);

        const auto wide_string = std::wstring(module_name.begin(), module_name.end());
        const auto module_base = get_module_address(process_id, wide_string.c_str());
        const auto module_size = get_module_size(process_id, wide_string.c_str());

        if (!module_base)
            return {};

        const auto pattern_bytes{ pattern_to_bytes(pattern) };

        MEMORY_BASIC_INFORMATION mbi{};

        VirtualQueryEx(process_handle, (LPCVOID)module_base, &mbi, sizeof(mbi));

        for (uintptr_t region_address = module_base; region_address < module_base + module_size; region_address += mbi.RegionSize)
        {
            if (!VirtualQueryEx(process_handle, reinterpret_cast<void*>(region_address), &mbi, sizeof(mbi)))
                continue;

            if (mbi.State != MEM_COMMIT || mbi.Protect == PAGE_NOACCESS)
                continue;

            for (uintptr_t current_address = region_address; region_address < module_base + mbi.RegionSize; current_address++)
            {
                bool at_current_address{ true };
                for (size_t idx{}; idx < pattern_bytes.size(); idx++)
                {
                    uint8_t byte{ read_vm<uint8_t>(process_handle, current_address + idx) };

                    if (pattern_bytes[idx] != (-1) && pattern_bytes[idx] != byte)
                    {
                        at_current_address = false;
                        break;
                    }
                }

                if (at_current_address)
                    return current_address;
            }
        }

        return {};
    }

    uintptr_t find_with_multiple_patterns(const HANDLE process_handle, const std::string_view module_name, std::vector<std::string_view> patterns)
    {
        for (auto& pattern : patterns)
        {
            if (auto match = find_pattern(process_handle, module_name, pattern))
                return match;
        }
    }
}