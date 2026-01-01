#include <fstream>
#include <iostream>
#include <imgui.h>
#include "../runtime_widgets/Widget.h"
#include "RuntimeWidgetManager.h"
#include "RuntimeWidgetFactory.h"

namespace rn {
    void RuntimeWidgetManager::UpdateAll() {
        for (size_t i = 0; i < widgets_.size(); i++) {
            widgets_[i]->UpdateInteraction((int)i);
        }
    }

    void RuntimeWidgetManager::RenderAll(ImDrawList* draw_list) {
        for (auto& widget : widgets_) {
            widget->Render(draw_list);
        }
    }

    void RuntimeWidgetManager::RenderContentAll() {
        for (auto& widget : widgets_) {

            ImVec2 screen_min = widget->GetScreenMin();
            ImVec2 screen_max = widget->GetScreenMax();

            widget->RenderContent(screen_min, screen_max);
        }
    }

    Widget* RuntimeWidgetManager::GetSelectedWidget() const {
        for (auto& widget : widgets_) {
            if (widget->IsSelected()) {
                return widget.get();
            }
        }
        return nullptr;
    }

    void RuntimeWidgetManager::DeleteWidget(const std::string& id) {
        for (auto it = widgets_.begin(); it != widgets_.end(); ++it) {
            if ((*it)->GetId() == id) {
                widgets_.erase(it);
                break;
            }
        }
    }

    

    void RuntimeWidgetManager::LoadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (file.is_open()) {
            nlohmann::json json;
            file >> json;
            FromJson(json);
        }
    }

    
    void RuntimeWidgetManager::FromJson(const nlohmann::json& json) {
        widgets_.clear();

        if (!json.contains("widgets") || !json["widgets"].is_array()) {
            return;
        }

        for (const auto& widget_json : json["widgets"]) {
            // Пробуем создать виджет через фабрику
            std::unique_ptr<Widget> widget = RuntimeWidgetFactory::CreateFromJson(widget_json);

            if (widget) {
                // Виджет уже создан и FromJson уже вызван внутри фабрики
                widgets_.push_back(std::move(widget));
            }
            else {
                std::cerr << "Фабрика не смогла создать (неверный формат или не зарегистрирован)";

            }
        }
    }
}