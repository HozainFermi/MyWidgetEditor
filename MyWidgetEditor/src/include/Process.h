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
	Process(const Process&) = delete;
	Process& operator=(const Process&) = delete;

#ifdef _WIN32
	Process(HANDLE handle, std::string config_name)
		: name(std::move(config_name)), hProcess(handle) {
	}
	Process(Process&& o) noexcept
		: name(std::move(o.name)), hProcess(o.hProcess) {
		o.hProcess = nullptr;
	}
	Process& operator=(Process&& o) noexcept {
		if (this != &o) {
			if (hProcess)
				CloseHandle(hProcess);
			name = std::move(o.name);
			hProcess = o.hProcess;
			o.hProcess = nullptr;
		}
		return *this;
	}
#else
	Process(pid_t pid_in, std::string config_name)
		: name(std::move(config_name)), pid(pid_in) {
	}
	Process(Process&& o) noexcept
		: name(std::move(o.name)), pid(o.pid) {
		o.pid = -1;
	}
	Process& operator=(Process&& o) noexcept {
		if (this != &o) {
			name = std::move(o.name);
			pid = o.pid;
			o.pid = -1;
		}
		return *this;
	}
#endif
	std::string GetName() const {
		return name;
	}

    ~Process() {
#ifdef _WIN32
        if (hProcess) {
			TerminateProcess(hProcess,0);
            CloseHandle(hProcess);
            hProcess = nullptr;
        }
#else
        if (pid > 0) {
            kill(pid, SIGTERM);            
        }
#endif
    }
};
