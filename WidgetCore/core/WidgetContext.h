#pragma once
#include <map>
#include <string>

namespace core {

    struct WindowProperties;
    class EventBus;

    // Где и как сейчас работают виджеты. Один объект на менеджер, виджеты хранят указатель
    struct WidgetContext {
        // true - виджет показан на канвасе редактора, false - запущен в Drawer
        bool is_editor = false;
        // Только для редактора: false - превью-данные, true - живые данные (HTTP и т.п.)
        bool live_data = true;
        // Свойства окна (цвет фона, шейдер). Может быть nullptr
        const WindowProperties* window = nullptr;
        // Куда Emit отправляет сообщения. Без шины Emit ничего не делает
        EventBus* bus = nullptr;
        // uniform-ы фонового шейдера окна: имя без "u_" -> значение. Пишут узлы ShaderParams,
        // окно каждый кадр передаёт их в шейдер как uniform float u_<имя>. Может быть nullptr
        std::map<std::string, float>* shader_uniforms = nullptr;
    };
}
