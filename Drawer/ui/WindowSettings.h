#pragma once
#include <GLFW/glfw3.h>
#include "../ui/RuntimeWindowProperties.h"
#include <windows.h>
#include <GLFW/glfw3.h>

#define GLFW_EXPOSE_NATIVE_WIN32
#include <GLFW/glfw3native.h>

#include <dwmapi.h>
#pragma comment(lib, "dwmapi.lib")

namespace rn {

	inline void SetGLFWHints(RuntimeWindowProperties& props) {
			
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

    bool IsWindows11OrGreater() {
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
#endif

    void SetupWindowStyle(GLFWwindow* window, RuntimeWindowProperties& props) {
#ifdef _WIN32


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

#endif
    }



}


