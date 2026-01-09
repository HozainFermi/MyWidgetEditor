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
    static bool launch(const std::string& command) {
#ifdef _WIN32
        return launchWindows(command);
#else
        return launchUnix(command);
#endif
    }

    /**
     * @brief Запускает исполняемый файл с аргументами
     * @param executablePath Путь к исполняемому файлу
     * @param arguments Аргументы командной строки
     * @return true если процесс успешно запущен
     */
    static bool launch(const std::string& executablePath, const std::string& arguments) {
#ifdef _WIN32
        //return launchWindowsExeArgs(executablePath, arguments);
        return launchWindowsShell(executablePath, arguments);
#else
        std::string command = "\"" + executablePath + "\" " + arguments;
        return launchUnix(command);
#endif
    }

private:
#ifdef _WIN32
    // Метод 1: Использование CreateProcess с правильными параметрами
    static bool launchWindows(const std::string& command) {
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
            CloseHandle(pi.hProcess);
            return true;
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

            return false;
        }
    }

    //Альтернативный способ через ShellExecute (проще)
    static bool launchWindowsShell(const std::string& executablePath, const std::string& arguments) {
        HINSTANCE result = ShellExecuteA(
            NULL,           // hwnd
            "open",         // operation
            executablePath.c_str(), // file
            arguments.c_str(), // parameters
            NULL,           // directory
            SW_HIDE         // show command
        );

        return ((intptr_t)result > 32);
    }

    // Раздельная передача exe и аргументов
    static bool launchWindowsExeArgs(const std::string& executablePath, const std::string& arguments) {
        STARTUPINFOA si = { sizeof(STARTUPINFOA) };
        PROCESS_INFORMATION pi = { 0 };

        // Формируем полную командную строку
        std::string fullCmd = "\"" + executablePath + "\" " + arguments;
        char* cmd = _strdup(fullCmd.c_str());

        bool success = CreateProcessA(
            NULL,           // lpApplicationName - используем командную строку
            cmd,            // lpCommandLine
            NULL,
            NULL,
            FALSE,
            CREATE_NEW_CONSOLE | DETACHED_PROCESS,
            NULL,
            NULL,
            &si,
            &pi
        );

        free(cmd);

        if (success) {
            CloseHandle(pi.hThread);
            CloseHandle(pi.hProcess);
            return true;
        }

        return false;
    }


#else
    static bool launchUnix(const std::string& command) {
        // ... существующий Unix код ...
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
            return true;
        }

        return false;
    }
#endif
};

#endif // INDEPENDENT_LAUNCHER_H