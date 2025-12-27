#include "WidgetManager.h"
#include "../widgets/TextWidget.h"
#include "../widgets/InputTextWidget.h"
#include <fstream>
#include <iostream>

namespace wg {
    void WidgetManager::UpdateAll(const ImVec2& canvas_p0, const ImVec2& canvas_size) {
        for (size_t i = 0; i < widgets_.size(); i++) {
            widgets_[i]->UpdateInteraction(canvas_p0,canvas_size, (int)i);            
        }
    }

    void WidgetManager::RenderAll(ImDrawList* draw_list, const ImVec2& canvas_p0) {
        for (auto& widget : widgets_) {
            widget->Render(draw_list, canvas_p0);
        }
    }

    void WidgetManager::RenderContentAll(const ImVec2& canvas_p0) {
        for (auto& widget : widgets_) {
            
            ImVec2 screen_min = widget->GetScreenMin(canvas_p0);
            ImVec2 screen_max = widget->GetScreenMax(canvas_p0); 

            widget->RenderContent(screen_min, screen_max);
        }
    }

    Widget* WidgetManager::GetSelectedWidget() const {
        for (auto& widget : widgets_) {
            if (widget->IsSelected()) {
                return widget.get();
            }
        }
        return nullptr;
    }

    void WidgetManager::DeleteWidget(const std::string& id) {
        for (auto it = widgets_.begin(); it != widgets_.end(); ++it) {
            if ((*it)->GetId() == id) {
                widgets_.erase(it);
                break;
            }
        }
    }

    void WidgetManager::SaveToFile(const std::string& filename) const {
        nlohmann::json json = ToJson();
        std::ofstream file(filename);
        if (file.is_open()) {
            file << json.dump(4);
        }
    }

    void WidgetManager::LoadFromFile(const std::string& filename) {
        std::ifstream file(filename);
        if (file.is_open()) {
            nlohmann::json json;
            file >> json;
            FromJson(json);
        }
    }

    nlohmann::json WidgetManager::ToJson() const {
        nlohmann::json json_array = nlohmann::json::array();
        for (const auto& widget : widgets_) {
            json_array.push_back(widget->ToJson());
        }
        return { {"widgets", json_array} };
    }

    void WidgetManager::FromJson(const nlohmann::json& json) {
        widgets_.clear();

        if (!json.contains("widgets") || !json["widgets"].is_array()) {
            return;
        }

        for (const auto& widget_json : json["widgets"]) {
            // Пробуем создать виджет через фабрику
            std::unique_ptr<Widget> widget = WidgetFactory::CreateFromJson(widget_json);

            if (widget) {
                // Виджет уже создан и FromJson уже вызван внутри фабрики
                widgets_.push_back(std::move(widget));
            }
            else {
                std::cout << "Фабрика не смогла создать (старый формат или не зарегистрирован)";
                
            }
        }
    }
}