#pragma once
#include <cstdint>
#include <string>

#ifdef _WIN32
#ifndef NOMINMAX
#define NOMINMAX
#endif
#include <windows.h>
#include <filesystem>
#else
#include <signal.h>
#include <sys/types.h>
#include <unistd.h>
#endif

// Запущенный виджет (процесс Drawer).
// Объект НЕ убивает процесс при уничтожении: виджеты живут и после закрытия редактора.
// Закрыть виджет - явно через Kill().
class Process {
public:
#ifdef _WIN32
    // Только что запущенный процесс (handle из CreateProcess)
    Process(HANDLE handle, std::string config_name)
        : name_(std::move(config_name)), handle_(handle) {
        pid_ = GetProcessId(handle_);
        created_ = CreationTime(handle_);
    }

    // Подхватить процесс, сохранённый в прошлый запуск редактора.
    // Вернёт false, если процесс уже завершился или pid теперь у другой программы
    static bool Attach(uint32_t pid, uint64_t created, std::string config_name, Process& out) {
        HANDLE handle = OpenProcess(PROCESS_TERMINATE | PROCESS_QUERY_LIMITED_INFORMATION | SYNCHRONIZE, FALSE, pid);
        if (!handle) {
            return false;
        }
        // Windows переиспользует pid: проверяем, что это наш Drawer, запущенный в то же время
        bool same = WaitForSingleObject(handle, 0) == WAIT_TIMEOUT &&
            CreationTime(handle) == created &&
            _wcsicmp(ExeName(handle).c_str(), L"Drawer.exe") == 0;
        if (!same) {
            CloseHandle(handle);
            return false;
        }
        out = Process(handle, std::move(config_name));
        return true;
    }

    bool IsRunning() const {
        return handle_ && WaitForSingleObject(handle_, 0) == WAIT_TIMEOUT;
    }

    void Kill() {
        if (handle_) {
            TerminateProcess(handle_, 0);
            WaitForSingleObject(handle_, 1000);
        }
    }
#else
    Process(pid_t pid, std::string config_name)
        : name_(std::move(config_name)), pid_((uint32_t)pid) {
    }

    static bool Attach(uint32_t pid, uint64_t, std::string config_name, Process& out) {
        if (kill((pid_t)pid, 0) != 0) {
            return false;
        }
        out = Process((pid_t)pid, std::move(config_name));
        return true;
    }

    bool IsRunning() const { return pid_ > 0 && kill((pid_t)pid_, 0) == 0; }
    void Kill() { if (pid_ > 0) kill((pid_t)pid_, SIGTERM); }
#endif

    Process() = default;
    Process(const Process&) = delete;
    Process& operator=(const Process&) = delete;
    Process(Process&& other) noexcept { *this = std::move(other); }
    Process& operator=(Process&& other) noexcept {
        if (this != &other) {
            CloseHandleIfAny();
            name_ = std::move(other.name_);
            pid_ = other.pid_;
            created_ = other.created_;
#ifdef _WIN32
            handle_ = other.handle_;
            other.handle_ = nullptr;
#endif
            other.pid_ = 0;
        }
        return *this;
    }

    ~Process() {
        CloseHandleIfAny();  // процесс продолжает работать
    }

    const std::string& GetName() const { return name_; }
    uint32_t GetPid() const { return pid_; }
    uint64_t GetCreationTime() const { return created_; }

private:
    void CloseHandleIfAny() {
#ifdef _WIN32
        if (handle_) {
            CloseHandle(handle_);
            handle_ = nullptr;
        }
#endif
    }

#ifdef _WIN32
    static uint64_t CreationTime(HANDLE handle) {
        FILETIME created{}, exited{}, kernel{}, user{};
        if (!GetProcessTimes(handle, &created, &exited, &kernel, &user)) {
            return 0;
        }
        return (uint64_t(created.dwHighDateTime) << 32) | created.dwLowDateTime;
    }

    static std::wstring ExeName(HANDLE handle) {
        wchar_t path[MAX_PATH];
        DWORD size = MAX_PATH;
        if (!QueryFullProcessImageNameW(handle, 0, path, &size)) {
            return {};
        }
        return std::filesystem::path(path).filename().wstring();
    }

    HANDLE handle_ = nullptr;
#endif
    std::string name_;
    uint32_t pid_ = 0;
    uint64_t created_ = 0;
};
