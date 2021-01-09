#pragma once
#include <Windows.h>
#include <Psapi.h>
#include <tlhelp32.h>
#include <vector>
#include <string>

namespace winapi
{
    //constexpr const uint8_t MAX_PATH = MAX_PATH;

    struct module_info
    {
        void* base_of_dll;
        uint32_t size_of_image;
        void* entrypoint;
    };

    struct process_entry32
    {
        uint32_t size;
        uint32_t usage_count;
        uint32_t process_id;
        uint64_t* default_heap_id;
        uint32_t module_info;
        uint32_t thread_count;
        uint32_t parent_process_id;
        long class_base;
        uint32_t flags;
        wchar_t file[MAX_PATH]; // TODO: constexpr MAX_PATH
    };

    class process
    {
    public:
        static inline void* open_process(int access, bool inherit_handle, int process_id)
        {
            return OpenProcess(
                access,
                inherit_handle,
                process_id);
        }
        static inline std::vector<void*> enum_process_modules(void* process)
        {
            HMODULE modules[1024];
            DWORD needed;
            if (EnumProcessModules(
                process,
                modules,
                sizeof(modules),
                &needed))
            {
                int size = sizeof(modules) / sizeof(modules[0]);
                return std::vector<void*>(
                    modules,
                    modules + size);
            }

            return std::vector<void*>();
        }
        static inline std::vector<int> enum_processes()
        {
            DWORD processes[1024];
            DWORD needed;
            if (EnumProcesses(
                processes,
                sizeof(processes),
                &needed))
            {
                int size = sizeof(processes) / sizeof(processes[0]);
                return std::vector<int>(
                    processes,
                    processes + size);
            }

            return std::vector<int>();
        }
        static inline void* create_toolhelp32_snapshot(int flags, int process_id)
        {
            return CreateToolhelp32Snapshot(flags, process_id);
        }
        static inline bool process32_first(void* snapshot, winapi::module_info* entry)
        {
            return Process32First(snapshot, (PROCESSENTRY32W*)entry);
        }
        static inline bool process32_next(void* snapshot, PROCESSENTRY32* entry)
        {
            return Process32Next(snapshot, entry);
        }
        static inline void sleep(uint32_t milliseconds)
        {
            Sleep(milliseconds);
        }
    };

    class module
    {
    public:
        static inline std::string get_module_file_name_ex(void* process, void* module)
        {
            TCHAR name[MAX_PATH];
            if (GetModuleFileNameEx(
                process,
                (HMODULE)module,
                name,
                sizeof(name) / sizeof(TCHAR)))
            {
                std::wstring wname = std::wstring(name);
                return std::string(wname.begin(), wname.end());
            }

            throw std::exception("GetModuleFileNameEx failed");
        }
        static inline winapi::module_info get_module_information(void* process, void* module)
        {
            MODULEINFO info = {};
            if (GetModuleInformation(
                process,
                (HMODULE)module,
                &info,
                sizeof(MODULEINFO)))
            {
                return module_info
                {
                    info.lpBaseOfDll,
                    info.SizeOfImage,
                    info.EntryPoint
                };
            }

            return module_info
            {
                nullptr,
                0,
                nullptr
            };
        }
        static inline std::string get_module_base_name(void* process, void* module)
        {
            TCHAR name[MAX_PATH] = L"";
            if (GetModuleBaseName(
                process,
                (HMODULE)module,
                name,
                sizeof(name) / sizeof(TCHAR)))
            {
                std::wstring wname = std::wstring(name);
                return std::string(wname.begin(), wname.end());
            }

            throw std::exception("GetModuleBaseName failed");
        }
    };

    class memory
    {
    public:
        static inline std::vector<uint8_t> read_process_memory(void* process, void* address, int size)
        {
            uint8_t* buffer = (uint8_t*)malloc(size);
            SIZE_T bytesRead;
            if (ReadProcessMemory(
                process,
                address,
                buffer,
                size,
                &bytesRead))
            {
                std::vector<uint8_t> memory(buffer, buffer + bytesRead);
                free(buffer);
                return memory;
            }

            return std::vector<uint8_t>();
        }
        static inline bool write_process_memory(void* process, int address, std::vector<uint8_t> data)
        {
            SIZE_T bytesWritten;
            return WriteProcessMemory(
                process,
                (void*)address,
                std::data(data),
                data.size(),
                &bytesWritten);
        }
    };

    class handle
    {
    public:
        static inline void close_handle(void* handle)
        {
            CloseHandle(handle);
        }
        static inline uint32_t wait_for_single_object(void* handle, uint32_t milliseconds)
        {
            return WaitForSingleObject(
                handle,
                milliseconds);
        }
    };

    class event
    {
    public:
        static inline void* open_event(int access, bool inherit_handle, std::wstring name)
        {
            return OpenEvent(
                access,
                inherit_handle,
                name.data());
        }
        static inline void* create_event(LPSECURITY_ATTRIBUTES event_attributes, bool manual_reset, bool initial_state, std::wstring name)
        {
            return CreateEvent(
                event_attributes,
                manual_reset,
                initial_state,
                name.data());
        }
        static inline bool set_event(void* event_handle)
        {
            return SetEvent(event_handle);
        }
    };

    class file
    {
    public:
        static inline void* open_file_mapping(int access, bool inherit_handle, std::wstring name)
        {
            return OpenFileMapping(access, inherit_handle, name.data());
        }
        static inline void* create_file_mapping(void* file, LPSECURITY_ATTRIBUTES attributes, uint32_t protect, uint32_t max_size_high, uint32_t max_size_low, std::wstring name)
        {
            return CreateFileMapping(
                file,
                attributes,
                protect,
                max_size_high,
                max_size_low,
                name.data());
        }
        static inline void* map_view_of_file(void* file, uint32_t access, uint32_t offset_high, uint32_t offset_low, int size)
        {
            return MapViewOfFile(
                file,
                access,
                offset_high,
                offset_low,
                size);
        }
        static inline bool unmap_view_of_file(void* file)
        {
            return UnmapViewOfFile(file);
        }
        static inline void* create_file(std::wstring file, uint32_t access, uint32_t shared, LPSECURITY_ATTRIBUTES security_attributes, uint32_t creation_disposition, uint32_t attributes, void* template_file)
        {
            return CreateFile(
                file.data(),
                access,
                shared,
                security_attributes,
                creation_disposition,
                attributes,
                template_file
            );
        }
    };
};