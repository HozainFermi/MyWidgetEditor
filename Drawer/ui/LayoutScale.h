#pragma once
#include <algorithm>
#include <GLFW/glfw3.h>
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <core/WindowProperties.h>

namespace rn {

    // Масштаб раскладки: окно (например, на весь экран) может быть больше холста редактора,
    // на котором раскладку делали (props.width x props.height).
    // ImGui считает интерфейс в единицах холста, а при выводе растягивает его на окно:
    // масштабируются виджеты, текст, графики и мышь. ImGui 1.92 растеризует шрифт под итоговый
    // масштаб, поэтому текст не мылится.
    struct LayoutScale {
        ImVec2 scale{ 1.0f, 1.0f };   // пикселей окна на единицу раскладки
        ImVec2 offset{ 0.0f, 0.0f };  // Fit: отступ, чтобы раскладка была по центру (в единицах раскладки)

        // Вызывать каждый кадр после ImGui_ImplGlfw_NewFrame() и до ImGui::NewFrame()
        void Apply(GLFWwindow* window, const core::WindowProperties& props) {
            scale = ImVec2(1.0f, 1.0f);
            offset = ImVec2(0.0f, 0.0f);

            int win_w = 0, win_h = 0, fb_w = 0, fb_h = 0;
            glfwGetWindowSize(window, &win_w, &win_h);
            glfwGetFramebufferSize(window, &fb_w, &fb_h);
            if (props.layout_scaling == core::LayoutScaling::Off || props.width <= 0 || props.height <= 0 || win_w <= 0 || win_h <= 0) {
                return;
            }

            float sx = win_w / (float)props.width;
            float sy = win_h / (float)props.height;
            if (props.layout_scaling == core::LayoutScaling::Fit) {
                sx = sy = (std::min)(sx, sy);
            }
            scale = ImVec2(sx, sy);

            ImGuiIO& io = ImGui::GetIO();
            io.DisplaySize = ImVec2(win_w / sx, win_h / sy);
            io.DisplayFramebufferScale = ImVec2(fb_w / io.DisplaySize.x, fb_h / io.DisplaySize.y);
            offset = ImVec2((io.DisplaySize.x - props.width) * 0.5f, (io.DisplaySize.y - props.height) * 0.5f);
        }

        // Точка окна (пиксели) -> единицы раскладки
        ImVec2 ToLayout(double x, double y) const {
            return ImVec2((float)x / scale.x, (float)y / scale.y);
        }
    };

    inline LayoutScale g_layout_scale;

    // Колбэк курсора ImGui с переводом координат в единицы раскладки
    inline void ScaledCursorPosCallback(GLFWwindow* window, double x, double y) {
        ImGui_ImplGlfw_CursorPosCallback(window, x / g_layout_scale.scale.x, y / g_layout_scale.scale.y);
    }

    // Вызывать после ImGui_ImplGlfw_InitForOpenGL(window, true)
    inline void InstallScaledCursorCallback(GLFWwindow* window) {
        glfwSetCursorPosCallback(window, ScaledCursorPosCallback);
    }
}
