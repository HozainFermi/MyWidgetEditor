#pragma once
#include <string>
#include <imgui.h>
#include <json.hpp>

namespace core {

    // Готовые режимы окна. Режим сам выставляет нужные флаги, вручную их в этом режиме не меняют
    enum class WindowMode {
        Normal = 0,     // обычное окно, флаги - как настроишь
        Wallpaper = 1,  // на рабочем столе под иконками, на весь экран
        Hud = 2         // поверх всего, на весь экран, клики проходят насквозь, фон прозрачный
    };

    // Свойства окна виджета (рантайм-окна Drawer). Редактор их настраивает, Drawer применяет
    struct WindowProperties {
        WindowMode mode = WindowMode::Normal;

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
        // Выставляет флаги, которые задаёт режим
        void ApplyMode();
        // Окно поверх всего, на весь экран и пропускает клики: с непрозрачным фоном оно закрыло бы
        // весь экран без возможности кликнуть, поэтому Drawer делает фон прозрачным
        bool IsClickThroughOverlay() const { return always_on_top && full_screen && mouse_passthrougth; }
    };

    nlohmann::json WindowPropertiesToJson(const WindowProperties& props);
    void WindowPropertiesFromJson(const nlohmann::json& json, WindowProperties& props);
}
