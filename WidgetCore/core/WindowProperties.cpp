#include "WindowProperties.h"
#include <algorithm>

namespace core {

    void WindowProperties::FloatToImU32() {
        bg_color = ImGui::ColorConvertFloat4ToU32(
            ImVec4(bg_color_float[0], bg_color_float[1], bg_color_float[2], bg_color_float[3]));
    }

    void WindowProperties::ApplyMode() {
        switch (mode) {
        case WindowMode::Wallpaper:
            wallpaper_mode = true;
            full_screen = true;
            always_on_top = false;
            always_on_bottom = false;
            mouse_passthrougth = false;
            decorated = false;
            resizeble = false;
            moveble = false;
            window_rounding = false;
            break;
        case WindowMode::Hud:
            wallpaper_mode = false;
            full_screen = true;
            always_on_top = true;
            always_on_bottom = false;
            mouse_passthrougth = true;
            decorated = false;
            resizeble = false;
            moveble = false;
            window_rounding = false;
            break;
        case WindowMode::Normal:
            wallpaper_mode = false;
            break;
        }
    }

    nlohmann::json WindowPropertiesToJson(const WindowProperties& props) {
        nlohmann::json window;
        window["mode"] = (int)props.mode;
        window["bg_color"] = nlohmann::json::array({
            props.bg_color_float[0], props.bg_color_float[1],
            props.bg_color_float[2], props.bg_color_float[3] });

        window["wallpaper_mode"] = props.wallpaper_mode;
        window["full_screen"] = props.full_screen;
        window["always_on_top"] = props.always_on_top;
        window["always_on_bottom"] = props.always_on_bottom;
        window["window_rounding"] = props.window_rounding;
        window["resizeble"] = props.resizeble;
        window["mouse_passthrougth"] = props.mouse_passthrougth;
        window["moveble"] = props.moveble;
        window["decorated"] = props.decorated;

        window["frag_shader"] = props.frag_GLSLshader_file;
        window["width"] = props.width;
        window["height"] = props.height;
        window["rounding"] = props.rounding;
        window["fps_limit"] = props.fps_limit;
        return window;
    }

    void WindowPropertiesFromJson(const nlohmann::json& window, WindowProperties& props) {
        if (window.contains("bg_color") && window["bg_color"].is_array() && window["bg_color"].size() >= 4) {
            for (int i = 0; i < 4; ++i) {
                props.bg_color_float[i] = window["bg_color"].at(i).get<float>();
            }
        }

        props.wallpaper_mode = window.value("wallpaper_mode", false);
        props.full_screen = window.value("full_screen", false);
        props.always_on_top = window.value("always_on_top", false);
        props.always_on_bottom = window.value("always_on_bottom", false);
        props.window_rounding = window.value("window_rounding", false);
        props.resizeble = window.value("resizeble", true);
        props.mouse_passthrougth = window.value("mouse_passthrougth", false);
        props.moveble = window.value("moveble", true);
        props.decorated = window.value("decorated", true);

        props.frag_GLSLshader_file = window.value("frag_shader", "");
        props.width = window.value("width", 150);
        props.height = window.value("height", 300);
        props.rounding = window.value("rounding", 0.0f);
        props.fps_limit = window.value("fps_limit", 60);

        // В старых конфигах режима нет - определяем по флагам
        if (window.contains("mode")) {
            props.mode = (WindowMode)std::clamp(window["mode"].get<int>(), 0, 2);
        }
        else if (props.wallpaper_mode) {
            props.mode = WindowMode::Wallpaper;
        }
        else if (props.IsClickThroughOverlay()) {
            props.mode = WindowMode::Hud;
        }
        else {
            props.mode = WindowMode::Normal;
        }
        props.ApplyMode();
        props.FloatToImU32();
    }
}
