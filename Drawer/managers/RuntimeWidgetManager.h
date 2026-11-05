#pragma once
#include <memory>
#include <string>
#include <vector>
#include <imgui.h>
#include <core/EventBus.h>
#include <core/Ports.h>
#include <core/Widget.h>
#include <core/WidgetContext.h>
#include <core/WindowProperties.h>

namespace rn {

    // Виджеты запущенного конфига: загрузка, связи и отрисовка
    class RuntimeWidgetManager {
    public:
        static RuntimeWidgetManager* Get() {
            static RuntimeWidgetManager instance;
            return &instance;
        }
        RuntimeWidgetManager(const RuntimeWidgetManager&) = delete;
        RuntimeWidgetManager& operator=(const RuntimeWidgetManager&) = delete;

        core::WindowProperties window_props_;

        // Только свойства окна: читаются до создания окна и GL-контекста
        bool WindowPropsFromJson(const std::string& filename);
        // Виджеты и связи: нужен готовый GL-контекст
        bool LoadFromFile(const std::string& filename);

        // mouse - курсор в координатах окна (для прозрачности при наведении), dt - секунды с прошлого кадра
        void UpdateAll(const ImVec2& origin, const ImVec2& mouse, float dt);
        void RenderAll(ImDrawList* draw_list, const ImVec2& origin);
        void RenderContentAll(const ImVec2& origin);

        size_t GetCount() const { return widgets_.size(); }
        // Наибольший FPS, который нужен виджетам без действий пользователя
        float GetDesiredFps() const;

    private:
        RuntimeWidgetManager();

        std::vector<std::unique_ptr<core::Widget>> widgets_;
        std::vector<core::Connection> connections_;
        core::EventBus bus_;
        core::WidgetContext context_;
    };
}
