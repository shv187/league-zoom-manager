#include "MemoryManagement.h"

#include "exceptions.h"

#include <vector>
#include <sstream>
#include <iostream>

#include <windows.h>
#include <tlhelp32.h> 

namespace memory_management
{
    const module_t get_module_data(DWORD process_id, const wchar_t* module_name)
    {
        auto snapshot_handle = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE | TH32CS_SNAPMODULE32, process_id);
        if (snapshot_handle == INVALID_HANDLE_VALUE)
            THROW_EXCEPTION("Failed while getting snapshot of modules");

        MODULEENTRY32 module_entry{};
        module_entry.dwSize = sizeof(module_entry);

        module_t module_data{};
        if (Module32First(snapshot_handle, &module_entry))
        {
            do
            {
                if (wcscmp(module_entry.szModule, module_name) == 0)
                {
                    module_data.base_address = reinterpret_cast<uintptr_t>(module_entry.modBaseAddr);
                    module_data.size = module_entry.modBaseSize;
                    break;
                }
            } while (Module32Next(snapshot_handle, &module_entry));
        }

        CloseHandle(snapshot_handle);

        return module_data;
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

    uintptr_t find_pattern(const HANDLE process_handle, const std::wstring_view module_name, const std::string_view pattern)
    {
        const auto module_data = get_module_data(get_process_id_from_process_handle(process_handle), module_name.data());

        if (!module_data.base_address || !module_data.size)
            return {};

        std::cout << std::format("Finding {}\n", pattern);

        const auto pattern_bytes{ pattern_to_bytes(pattern) };

        const auto dos_header = read_vm<IMAGE_DOS_HEADER>(process_handle, module_data.base_address);
        const auto nt_headers = read_vm<IMAGE_NT_HEADERS>(process_handle, module_data.base_address + dos_header.e_lfanew);

        const auto _text_offset = nt_headers.OptionalHeader.BaseOfCode;
        const auto _text_size = nt_headers.OptionalHeader.SizeOfCode;

        MEMORY_BASIC_INFORMATION mbi{};

        for (uintptr_t region_address = module_data.base_address + _text_offset; region_address < module_data.base_address + _text_offset + _text_size - pattern_bytes.size(); region_address += (mbi.RegionSize + 1))
        {
            if (!VirtualQueryEx(process_handle, reinterpret_cast<void*>(region_address), &mbi, sizeof(mbi)))
                continue;

            if (mbi.State != MEM_COMMIT || mbi.Protect == PAGE_NOACCESS)
                continue;

            for (uintptr_t current_address = region_address; region_address < reinterpret_cast<uintptr_t>(mbi.BaseAddress) + mbi.RegionSize; current_address++)
            {
                bool at_current_address{ true };

                std::vector<uint8_t> buffer(pattern_bytes.size());
                ReadProcessMemory(process_handle, (void*)current_address, buffer.data(), pattern_bytes.size(), 0);

                auto compare = [&](const std::vector<int>& pattern_vector, const std::vector<uint8_t>& buffer) {
                    for (size_t i = 0; i < pattern_vector.size(); i++)
                    {
                        if (pattern_vector[i] != buffer[i] && pattern_vector[i] != -1)
                            return false;
                    }

                    return true;
                };

                if (!compare(pattern_bytes, buffer))
                {
                    at_current_address = false;
                    continue;
                }

                if (at_current_address)
                    return current_address;
            }
        }

        return {};
    }

    uintptr_t find_with_multiple_patterns(const HANDLE process_handle, const std::wstring_view module_name, std::vector<std::string_view> patterns)
    {
        for (auto& pattern : patterns)
        {
            if (auto match = find_pattern(process_handle, module_name, pattern))
                return match;
        }
    }
}