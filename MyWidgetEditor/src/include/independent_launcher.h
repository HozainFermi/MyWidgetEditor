#ifndef INDEPENDENT_LAUNCHER_H
#define INDEPENDENT_LAUNCHER_H

#include <string>
#include <iostream>

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

class IndependentLauncher {
public:

    /**
     * @brief Запускает процесс полностью независимо от родительского
     * @param command Полная командная строка (исполняемый файл + аргументы)
     * @return true если процесс успешно запущен
     */

#ifdef _WIN32
    static HANDLE launch(const std::string& command) {
        return launchWindows(command);        
    }
#else
    static pid_t launch(const std::string& command) {
        return launchUnix(command);
    }
#endif
    /**
     * @brief Запускает исполняемый файл с аргументами
     * @param executablePath Путь к исполняемому файлу
     * @param arguments Аргументы командной строки
     * @return true если процесс успешно запущен
     */
#ifdef _WIN32
    static HANDLE launch(const std::string& executablePath, const std::string& arguments) {
        //return launchWindowsExeArgs(executablePath, arguments);
        return LaunchWithExeWorkingDir(executablePath, arguments);
    }
#else
    static pid_t launch(const std::string& executablePath, const std::string& arguments) {
        std::string command = "\"" + executablePath + "\" " + arguments;
        return launchUnix(command);
    }
#endif


private:
#ifdef _WIN32
    //Использование CreateProcess с правильными параметрами
    static HANDLE launchWindows(const std::string& command) {

        STARTUPINFOA si = { sizeof(STARTUPINFOA) };
        PROCESS_INFORMATION pi = { 0 };

        // Создаем копию строки, так как CreateProcess требует изменяемую строку
        char* cmd = _strdup(command.c_str());

        bool success = CreateProcessA(
            NULL,           // lpApplicationName - NULL, тогда используется lpCommandLine
            cmd,            // lpCommandLine - командная строка
            NULL,           // Атрибуты процесса
            NULL,           // Атрибуты потока
            FALSE,          // Наследование дескрипторов
            CREATE_NEW_CONSOLE | DETACHED_PROCESS, // Флаги
            NULL,           // Окружение
            NULL,           // Текущий каталог
            &si,            // STARTUPINFO
            &pi             // PROCESS_INFORMATION
        );

        free(cmd);

        if (success) {
            CloseHandle(pi.hThread);
            //CloseHandle(pi.hProcess);
            return pi.hProcess;
        }
        else {
            DWORD error = GetLastError();
            std::cerr << "CreateProcess failed. Error " << error << ": ";

            // Расшифровка ошибки
            switch (error) {
            case 2: std::cerr << "ERROR_FILE_NOT_FOUND"; break;
            case 3: std::cerr << "ERROR_PATH_NOT_FOUND"; break;
            case 5: std::cerr << "ERROR_ACCESS_DENIED"; break;
            case 87: std::cerr << "ERROR_INVALID_PARAMETER"; break;
            case 123: std::cerr << "ERROR_INVALID_NAME"; break;
            case 267: std::cerr << "ERROR_DIRECTORY"; break;
            default: std::cerr << "Unknown error"; break;
            }
            std::cerr << std::endl;

            return pi.hProcess;
        }
    }

    //Альтернативный способ через ShellExecute 
    static HANDLE launchWindowsShellExA(const std::string& executablePath, const std::string& arguments) {
                
        HANDLE handle{0};

        HINSTANCE result = ShellExecuteA(
            NULL,           // hwnd
            "open",         // operation
            executablePath.c_str(), // file
            arguments.c_str(), // parameters
            NULL,           // directory
            SW_HIDE         // show command
        );

        //return ((intptr_t)result > 32);
        return handle;
    }

    static std::string StripOuterQuotes(std::string s) {     
        
        if (s.size() >= 2 && s.front() == '"' && s.back() == '"') {
            s = s.substr(1, s.size() - 2);
        }
        return s;
    }

    static HANDLE LaunchWithExeWorkingDir(std::string exeQuotedOrNot, std::string argQuotedOrNot) {
        const std::string exe = StripOuterQuotes(exeQuotedOrNot);
                
        // рабочая директория = папка exe
        std::string workDir;
        const auto pos = exe.find_last_of("\\");
        if (pos != std::string::npos) {
            workDir = exe.substr(0, pos); 
        }
                
        std::string cmdLine = exeQuotedOrNot+" "+argQuotedOrNot;
        
        std::vector<char> cmdBuf(cmdLine.begin(), cmdLine.end());
        cmdBuf.push_back('\0');
        STARTUPINFOA si{};
        si.cb = sizeof(si);

        PROCESS_INFORMATION pi{};
        
        const BOOL ok = CreateProcessA(
            nullptr,
            cmdBuf.data(),
            nullptr, 
            nullptr,
            FALSE,
            CREATE_NEW_CONSOLE,
            nullptr,
            workDir.c_str(),
            &si,
            &pi
        );
                
        if (!ok) {
            DWORD e = GetLastError();
            std::cerr << "CreateProcess failed, GetLastError=" << e
                << "  exe=" << exeQuotedOrNot << "\n  arg=" << argQuotedOrNot << "\n  cwd=" << workDir << "\n";
            return nullptr;
        }
        CloseHandle(pi.hThread);
        return pi.hProcess; // закрой CloseHandle когда не нужен / после TerminateProcess
    }

    static HANDLE launchWindowsShellEx(const std::string& executablePath, const std::string& arguments) {
        // 1. Извлекаем путь к папке (рабочую директорию)
        std::string directory = executablePath.substr(0, executablePath.find_last_of("\\/"));

        // 2. Оборачиваем аргументы в кавычки на случай пробелов в путях
        std::string quotedArgs = "\"" + arguments + "\"";

        SHELLEXECUTEINFOA sei = { sizeof(sei) }; // Явно используем ANSI версию (A)
        sei.fMask = SEE_MASK_NOCLOSEPROCESS;
        sei.lpVerb = "open";
        sei.lpFile = executablePath.c_str();
        sei.lpParameters = quotedArgs.c_str();
        sei.lpDirectory = directory.c_str(); // Устанавливаем рабочую папку
        sei.nShow = SW_SHOWNORMAL;

        if (!ShellExecuteExA(&sei)) {
            DWORD dwError = GetLastError();
            std::cerr << "Failed to launch process. Error code: " << dwError << std::endl;
            return NULL;
        }

        // Если hProcess все еще NULL, значит приложение открылось через DDE 
        // (например, в существующей вкладке другого процесса)
        return sei.hProcess;
    }


    // Раздельная передача exe и аргументов
    static HANDLE launchWindowsExeArgs(const std::string& executablePath, const std::string& arguments) {
        STARTUPINFOA si = { sizeof(STARTUPINFOA) };
        PROCESS_INFORMATION pi = { 0 };

        // Формируем полную командную строку
        std::string fullCmd = executablePath +" "+ arguments;
        std::cout << std::endl <<fullCmd;
        char* cmd = _strdup(fullCmd.c_str());

        bool success = CreateProcessA(
            NULL,           // lpApplicationName - используем командную строку
            cmd,            // lpCommandLine
            NULL,
            NULL,
            FALSE,
            CREATE_NEW_CONSOLE,
            NULL,
            NULL,
            &si,
            &pi
        );

        free(cmd);

        if (success) {
            CloseHandle(pi.hThread);
            //CloseHandle(pi.hProcess);
            return pi.hProcess;
        }
        else {
            std::cerr << "Error code: " << GetLastError() << std::endl;
        }

        return pi.hProcess;
    }


#else
    static pid_t launchUnix(const std::string& command) {
        
        
        //!!!NOT DONE
        pid_t pid = fork();

        if (pid == 0) {
            // Дочерний процесс
            setsid();
            signal(SIGHUP, SIG_IGN);

            if (fork() == 0) {
                execl("/bin/sh", "sh", "-c", command.c_str(), (char*)NULL);
                _exit(EXIT_FAILURE);
            }
            _exit(EXIT_SUCCESS);
        }
        else if (pid > 0) {
            waitpid(pid, NULL, 0);
            return pid;
        }

        return pid;
    }
#endif
};

#endif