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
        if (json.contains("widgets") && json["widgets"].is_array()) {
            for (const auto& widget_json : json["widgets"]) {
                WidgetType type = static_cast<WidgetType>(widget_json.value("type", 0));
                std::string name = widget_json.value("name", "Unnamed");
                ImVec2 pos(0, 0);
                ImVec2 size(100, 50);

                if (widget_json.contains("position") && widget_json["position"].is_array()) {
                    pos = ImVec2(widget_json["position"][0], widget_json["position"][1]);
                }

                if (widget_json.contains("size") && widget_json["size"].is_array()) {
                    size = ImVec2(widget_json["size"][0], widget_json["size"][1]);
                }

                std::unique_ptr<Widget> widget;
                switch (type) {
                case WidgetType::TEXT:
                    widget = std::make_unique<TextWidget>(name, pos, "");
                    break;
                case WidgetType::INPUT_TEXT:
                    widget = std::make_unique<InputTextWidget>(name, pos);
                    break;
                    // ... другие типы
                }

                if (widget) {
                    widget->FromJson(widget_json);
                    widgets_.push_back(std::move(widget));
                }
            }
        }
    }
}