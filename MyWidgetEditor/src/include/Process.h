#pragma once
#ifdef _WIN32
#include <windows.h>
#include <shellapi.h> // для ShellExecute как альтернативы
#else
#include <unistd.h>
#include <sys/types.h>
#include <cstdlib>
#include <cstring>
#include <signal.h>
#endif

class Process {

    std::string name;
#ifdef _WIN32
    HANDLE hProcess;
#else
    pid_t pid;
#endif


public:
    std::string GetName() {
        return name;
    }

#ifdef _WIN32
    Process(HANDLE handle, std::string config_name) {
        hProcess = handle;
        name = config_name;
    }
#else
    Process(pid_t pid, std::string config_name) {
        this.pid = pid;
        name = config_name;
    }
#endif

    ~Process() {
#ifdef _WIN32
        if (hProcess != NULL) {
            TerminateProcess(hProcess, 0); // Принудительно закрываем
            CloseHandle(hProcess);         // ОБЯЗАТЕЛЬНО освобождаем хендл
        }
#else
        if (pid > 0) {
            kill(pid, SIGTERM);            // Аналог закрытия в Unix
        }
#endif
    }
};
