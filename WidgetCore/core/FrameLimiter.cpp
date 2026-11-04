#include "FrameLimiter.h"
#include <algorithm>
#include <GLFW/glfw3.h>

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#include <timeapi.h>
#pragma comment(lib, "winmm.lib")
#endif

namespace core {

    namespace {
        const auto kActiveTime = std::chrono::milliseconds(500);
    }

    FrameLimiter::FrameLimiter()
        : last_frame_(Clock::now()), active_until_(Clock::now() + kActiveTime) {
#ifdef _WIN32
        // По умолчанию таймер Windows тикает раз в ~15.6 мс, и сон на 16 мс превращается в 31
        timeBeginPeriod(1);
#endif
    }

    FrameLimiter::~FrameLimiter() {
#ifdef _WIN32
        timeEndPeriod(1);
#endif
    }

    void FrameLimiter::WaitForNextFrame(double idle_fps, double active_fps) {
        if (active_fps <= 0.0) {
            glfwPollEvents();
            last_frame_ = Clock::now();
            return;
        }
        idle_fps = std::clamp(idle_fps, 0.5, active_fps);

        while (true) {
            const auto now = Clock::now();
            const double fps = now < active_until_ ? active_fps : idle_fps;
            const auto deadline = last_frame_ + std::chrono::duration_cast<Clock::duration>(std::chrono::duration<double>(1.0 / fps));
            const double remaining = std::chrono::duration<double>(deadline - now).count();
            if (remaining <= 0.0) {
                glfwPollEvents();
                break;
            }

            glfwWaitEventsTimeout(remaining);
            // Проснулись заметно раньше срока - пришло событие (мышь, клавиатура, ресайз)
            if (Clock::now() + std::chrono::milliseconds(1) < deadline) {
                active_until_ = Clock::now() + kActiveTime;
            }
        }
        last_frame_ = Clock::now();
    }
}
