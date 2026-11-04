#pragma once

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
    };
}
