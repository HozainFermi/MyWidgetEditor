#include "RuntimeWidgetManager.h"
#include <iostream>
#include <core/Config.h>

namespace rn {

    RuntimeWidgetManager::RuntimeWidgetManager() {
        context_.is_editor = false;
        context_.live_data = true;
        context_.window = &window_props_;
        context_.bus = &bus_;
    }

    bool RuntimeWidgetManager::WindowPropsFromJson(const std::string& filename) {
        std::string error;
        if (!core::LoadWindowProperties(filename, window_props_, &error)) {
            std::cerr << error << std::endl;
            return false;
        }
        return true;
    }

    bool RuntimeWidgetManager::LoadFromFile(const std::string& filename) {
        core::ConfigData data;
        std::string error;
        if (!core::LoadConfig(filename, data, &error)) {
            std::cerr << error << std::endl;
            return false;
        }

        // Свойства окна оставляем прочитанными до создания окна: путь к вершинному шейдеру там уже задан
        widgets_ = std::move(data.widgets);
        connections_ = std::move(data.connections);

        std::vector<core::Widget*> widgets;
        for (auto& widget : widgets_) {
            widget->SetContext(&context_);
            widgets.push_back(widget.get());
        }
        bus_.SetRoutes(widgets, connections_);
        return true;
    }

    void RuntimeWidgetManager::UpdateAll() {
        for (auto& widget : widgets_) {
            widget->Update();
        }
        // Доставляем всё, что виджеты отправили (в том числе из фоновых потоков)
        bus_.Dispatch();
    }

    void RuntimeWidgetManager::RenderAll(ImDrawList* draw_list, const ImVec2& origin) {
        for (auto& widget : widgets_) {
            widget->Render(draw_list, widget->GetScreenMin(origin), widget->GetScreenMax(origin));
        }
    }

    void RuntimeWidgetManager::RenderContentAll(const ImVec2& origin) {
        for (auto& widget : widgets_) {
            widget->RenderContent(widget->GetScreenMin(origin), widget->GetScreenMax(origin));
        }
    }
}
