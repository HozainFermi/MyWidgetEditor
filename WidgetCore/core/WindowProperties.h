#pragma once
#include <string>
#include <imgui.h>
#include <json.hpp>

namespace core {

    // Свойства окна виджета (рантайм-окна Drawer). Редактор их настраивает, Drawer применяет
    struct WindowProperties {
        ImU32 bg_color = IM_COL32(40, 40, 40, 255);
        float bg_color_float[4]{ 0.15686275f, 0.15686275f, 0.15686275f, 1.0f };

        bool wallpaper_mode = false;
        bool full_screen = false;
        bool always_on_top = false;
        bool always_on_bottom = false;
        bool window_rounding = false;
        bool resizeble = true;
        bool mouse_passthrougth = false;
        bool moveble = true;
        bool decorated = true;

        std::string vertex_GLSLshader_file = std::string(ASSETS_SOURCE_DIR) + "/shaders/base_vertex.vert";
        std::string frag_GLSLshader_file = "";
        int width = 500;
        int height = 500;
        float rounding = 0.0f;
        // Максимум кадров в секунду для Drawer (0 - без ограничения, только vsync)
        int fps_limit = 60;

        // Пересчитывает bg_color из bg_color_float
        void FloatToImU32();
    };

    nlohmann::json WindowPropertiesToJson(const WindowProperties& props);
    void WindowPropertiesFromJson(const nlohmann::json& json, WindowProperties& props);
}
