#pragma once
#include <GLFW/glfw3.h>
#include <core/WindowProperties.h>
#include <windows.h>
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

namespace rn {

	inline void SetGLFWHints(const core::WindowProperties& props) {
			
		glfwWindowHint(GLFW_DECORATED, props.decorated ? GLFW_TRUE : GLFW_FALSE);		
		glfwWindowHint(GLFW_FLOATING, props.always_on_top ? GLFW_TRUE : GLFW_FALSE);		
		glfwWindowHint(GLFW_MOUSE_PASSTHROUGH, props.mouse_passthrougth ? GLFW_TRUE : GLFW_FALSE);
		glfwWindowHint(GLFW_RESIZABLE, props.resizeble ? GLFW_TRUE : GLFW_FALSE);
	}

#ifdef _WIN32


    typedef struct _RTL_OSVERSIONINFOW {
        ULONG dwOSVersionInfoSize;
        ULONG dwMajorVersion;
        ULONG dwMinorVersion;
        ULONG dwBuildNumber;
        ULONG dwPlatformId;
        WCHAR szCSDVersion[128];
    } RTL_OSVERSIONINFOW, * PRTL_OSVERSIONINFOW;

    inline bool IsWindows11OrGreater() {
        typedef NTSTATUS(WINAPI* PfnRtlGetVersion)(PRTL_OSVERSIONINFOW);

        HMODULE hMod = GetModuleHandleW(L"ntdll.dll");
        if (hMod) {
            PfnRtlGetVersion RtlGetVersionFn = (PfnRtlGetVersion)GetProcAddress(hMod, "RtlGetVersion");

            if (RtlGetVersionFn != nullptr) {
                RTL_OSVERSIONINFOW rovi = { 0 };
                rovi.dwOSVersionInfoSize = sizeof(rovi);
                if (RtlGetVersionFn(&rovi) == 0) {
                    return rovi.dwBuildNumber >= 22000;
                }
            }
        }
        return false;
    }

    BOOL CALLBACK EnumWindowsProc(HWND hwnd, LPARAM lParam) {
        HWND p = FindWindowEx(hwnd, NULL, "SHELLDLL_DefView", NULL);
        if (p != NULL) {
            // Находим следующее окно после того, где лежат иконки
            *(HWND*)lParam = FindWindowEx(NULL, hwnd, "WorkerW", NULL);
        }
        return TRUE;
    }

    inline void ClearWindow(GLFWwindow* window) {
        if (!window) return;
        HWND hwnd = glfwGetWin32Window(window);

        ShowWindow(hwnd, SW_HIDE);
      
        SetParent(hwnd, NULL);
    
        HWND progman = FindWindowW(L"Progman", NULL);
        SendMessageTimeoutW(progman, 0x052C, 0, 0, SMTO_NORMAL, 1000, NULL);
        
        InvalidateRect(NULL, NULL, TRUE);
    }

    struct ScopedWindowCleaner {
        GLFWwindow* window;
        ScopedWindowCleaner(GLFWwindow* win) : window(win) {}
        ~ScopedWindowCleaner() {
            if (window) {
                ClearWindow(window);
            }
        }
    };

    // ===== Аварийное закрытие: Ctrl+Alt+Shift+Q =====
    // Нужно, когда виджет поверх всего и пропускает клики - иначе его не закрыть мышью.
    // Системный хоткей может занять только один процесс, поэтому остальные виджеты
    // проверяют клавиши сами (IsCloseHotkeyDown) - комбинация закрывает все запущенные виджеты.
    constexpr int kCloseHotkeyId = 0xC105;
    inline WNDPROC g_glfw_window_proc = nullptr;
    inline GLFWwindow* g_hotkey_window = nullptr;

    inline LRESULT CALLBACK CloseHotkeyWindowProc(HWND hwnd, UINT msg, WPARAM wparam, LPARAM lparam) {
        if (msg == WM_HOTKEY && wparam == kCloseHotkeyId && g_hotkey_window) {
            glfwSetWindowShouldClose(g_hotkey_window, GLFW_TRUE);
            return 0;
        }
        return CallWindowProcW(g_glfw_window_proc, hwnd, msg, wparam, lparam);
    }

    // true - хоткей наш; false - его уже занял другой виджет
    inline bool RegisterCloseHotkey(GLFWwindow* window) {
        HWND hwnd = glfwGetWin32Window(window);
        g_hotkey_window = window;
        // WM_HOTKEY приходит в оконную процедуру, а её обрабатывает GLFW - встаём перед ней
        g_glfw_window_proc = (WNDPROC)SetWindowLongPtrW(hwnd, GWLP_WNDPROC, (LONG_PTR)CloseHotkeyWindowProc);
        return RegisterHotKey(hwnd, kCloseHotkeyId, MOD_CONTROL | MOD_ALT | MOD_SHIFT | MOD_NOREPEAT, 'Q') != 0;
    }

    inline bool IsCloseHotkeyDown() {
        auto down = [](int key) { return (GetAsyncKeyState(key) & 0x8000) != 0; };
        return down(VK_CONTROL) && down(VK_MENU) && down(VK_SHIFT) && down('Q');
    }


#endif

    inline void SetupWindowStyle(GLFWwindow* window, const core::WindowProperties& props) {
#ifdef _WIN32
        HWND hwnd = glfwGetWin32Window(window);
        
        if (props.window_rounding) {        
            if (IsWindows11OrGreater()) {            
                    int preference = 2; // DWMWCP_ROUND
                    DwmSetWindowAttribute(glfwGetWin32Window(window), 33, &preference, sizeof(preference));            
            }
            else {
                // Windows 10 и ниже: Системного закругления нет.            
                glfwWindowHint(GLFW_DECORATED, GLFW_FALSE);
            }        
        }
        if (props.wallpaper_mode) {
            //HWND hwnd = glfwGetWin32Window(window);
            //Убираем из таскбара
            LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
            exStyle &= ~WS_EX_APPWINDOW;
            exStyle |= WS_EX_TOOLWINDOW;   // Скрыть из панели задач        
            SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle);

            // создал WorkerW
            HWND progman = FindWindowW(L"Progman", NULL);
            SendMessageTimeoutW(progman, 0x052C, 0, 0, SMTO_NORMAL, 1000, NULL);

            // Ищем созданный WorkerW
            HWND workerw = NULL;
            EnumWindows(EnumWindowsProc, (LPARAM)&workerw);

            // Если WorkerW найден — встраиваемся В него
            HWND parent = workerw != NULL ? workerw : progman; // запасной вариант - Progman
            SetParent(hwnd, parent);

            // После SetParent координаты окна считаются от родителя (левый верхний угол всего
            // виртуального экрана), а не от монитора. Без этого окно остаётся там, где его
            // создал GLFW, и по краям видно обои.
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            const GLFWvidmode* mode = glfwGetVideoMode(monitor);
            int monitor_x = 0, monitor_y = 0;
            glfwGetMonitorPos(monitor, &monitor_x, &monitor_y);
            POINT origin{ monitor_x, monitor_y };
            ScreenToClient(parent, &origin);
            SetWindowPos(hwnd, NULL, origin.x, origin.y, mode->width, mode->height,
                SWP_NOZORDER | SWP_NOACTIVATE | SWP_FRAMECHANGED);
        }
        else if (props.full_screen) {
            GLFWmonitor* monitor = glfwGetPrimaryMonitor();
            int monitor_x = 0, monitor_y = 0;
            glfwGetMonitorPos(monitor, &monitor_x, &monitor_y);
            glfwSetWindowPos(window, monitor_x, monitor_y);
        }
        if (props.always_on_bottom) {
            //HWND hwnd = glfwGetWin32Window(window);

            //Убираем из таскбара и запрещаем активацию при клике
            LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
            exStyle &= ~WS_EX_APPWINDOW;
            exStyle |= WS_EX_TOOLWINDOW;   // Скрыть из панели задач
            exStyle |= WS_EX_NOACTIVATE;   // Не выводить на передний план при клике
            SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle);

            HWND progman = FindWindowW(L"Progman", NULL);
            SetWindowPos(hwnd, progman, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOACTIVATE | SWP_FRAMECHANGED);

        }
        if (props.always_on_top) {
            //Убираем из таскбара
            LONG_PTR exStyle = GetWindowLongPtr(hwnd, GWL_EXSTYLE);
            exStyle &= ~WS_EX_APPWINDOW;
            exStyle |= WS_EX_TOOLWINDOW;   // Скрыть из панели задач        
            SetWindowLongPtr(hwnd, GWL_EXSTYLE, exStyle);
        }
        
#endif
    }



}


