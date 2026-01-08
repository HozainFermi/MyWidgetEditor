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
        if (filename.empty()) {
            return;
        }
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
        if (!json.contains("window")) {
            return;
        }
        const nlohmann::json& windowjs = json["window"];

        window_props_.always_on_top = windowjs.value("always_on_top", false);
        if (windowjs.contains("bg_color") && windowjs["bg_color"].is_array()) {
            window_props_.bg_color_float[0] = windowjs["bg_color"].at(0).get<float>();
            window_props_.bg_color_float[1] = windowjs["bg_color"].at(1).get<float>();
            window_props_.bg_color_float[2] = windowjs["bg_color"].at(2).get<float>();
            window_props_.bg_color_float[3] = windowjs["bg_color"].at(3).get<float>();
        }
        window_props_.frag_GLSLshader_file = windowjs.value("frag_shader", "");
        window_props_.width = windowjs.value("width", 150);
        window_props_.height = windowjs.value("height", 300);
        window_props_.moveble = windowjs.value("moveble", true);
        window_props_.FloatToImU32();

        for (const auto& widget_json : json["widgets"]) {
            // ѕробуем создать виджет через фабрику
            std::unique_ptr<Widget> widget = RuntimeWidgetFactory::CreateFromJson(widget_json);

            if (widget) {
                
                widget.get()->FromJson(widget_json);
                widgets_.push_back(std::move(widget));
                std::cout << widgets_.size() <<std::endl;
            }
            else {
                std::cerr << "‘абрика не смогла создать (неверный формат или не зарегистрирован)";

            }
        }
    }
}