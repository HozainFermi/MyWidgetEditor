#pragma once
#include <chrono>

namespace core {

    // Ограничитель частоты кадров для цикла GLFW. Вызывать в начале каждого кадра вместо glfwPollEvents().
    // Между кадрами поток спит в glfwWaitEventsTimeout, поэтому процессор не крутится вхолостую.
    // Мышь/клавиатура будят сразу, и на полсекунды включается active_fps, чтобы интерфейс не тормозил.
    class FrameLimiter {
    public:
        FrameLimiter();
        ~FrameLimiter();

        // idle_fps - сколько кадров нужно без действий пользователя, active_fps - при действиях.
        // fps <= 0 - без ограничения (только vsync)
        void WaitForNextFrame(double idle_fps, double active_fps);

    private:
        using Clock = std::chrono::steady_clock;
        Clock::time_point last_frame_;
        Clock::time_point active_until_;
    };
}
