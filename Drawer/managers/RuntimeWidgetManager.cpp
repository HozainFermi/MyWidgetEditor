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
                
        //Создаём соединения
        if (json.contains("connections")) {
            for (const auto& connection : json["connections"])
            {
                connections_.push_back(Connection{
                     PortRef{connection["from_port"],connection["from_widget"]},
                     PortRef{connection["to_port"],connection["to_widget"]}
                    });
            }

        }
        
        // Пробуем создать виджет через фабрику            
        for (const auto& widget_json : json["widgets"]) {
            std::unique_ptr<Widget> widget = RuntimeWidgetFactory::CreateFromJson(widget_json);

            if (widget) {                
                widget->FromJson(widget_json);
                widget->window_props = &window_props_;
                widgets_by_id_[widget->GetId()] = widget.get();
                widgets_.push_back(std::move(widget));                
            }
            else {std::cerr << "Фабрика не смогла создать (неверный формат или не зарегистрирован)";}            
        }

            // Добавляем OnInput связанного виджета в коллбеки текущего виджета                
            for (auto& widget : widgets_) {
                for (auto& connection : connections_) {
                if (connection.from.widget_id == widget->GetId()) {
                    Widget* target_widget = widgets_by_id_[connection.to.widget_id];

                    if (target_widget) {
                        // Сохраняем лямбду, которая вызывает метод OnInput у target_widget
                        widget->from_port_callbacks_[connection.from.port] =
                            [target_widget](const std::string from_widget_id, std::string from_port, const std::vector<WidgetValue>& data) {
                            target_widget->OnInput(from_widget_id, from_port, data);
                            };
                    }
                }
                }
            }
    
    
    }

    void RuntimeWidgetManager::WindowPropsFromJson(const std::string& filename)
    {
        nlohmann::json json;

        if (filename.empty()) {
            return;
        }
        std::ifstream file(filename);
        if (file.is_open()) {            
            file >> json;            
        }

        const nlohmann::json& windowjs = json["window"];

        if (windowjs.contains("bg_color") && windowjs["bg_color"].is_array()) {
            window_props_.bg_color_float[0] = windowjs["bg_color"].at(0).get<float>();
            window_props_.bg_color_float[1] = windowjs["bg_color"].at(1).get<float>();
            window_props_.bg_color_float[2] = windowjs["bg_color"].at(2).get<float>();
            window_props_.bg_color_float[3] = windowjs["bg_color"].at(3).get<float>();
        }

        window_props_.full_screen = windowjs.value("full_screen", false);
        window_props_.always_on_top = windowjs.value("always_on_top", false);
        window_props_.always_on_bottom = windowjs.value("always_on_bottom", false);
        window_props_.window_rounding = windowjs.value("window_rounding", false);
        window_props_.decorated = windowjs.value("decorated", true);
        window_props_.resizeble = windowjs.value("resizeble", true);
        window_props_.mouse_passthrougth = windowjs.value("mouse_passthrougth", false);
        window_props_.moveble = windowjs.value("moveble", true);

        window_props_.frag_GLSLshader_file = windowjs.value("frag_shader", "");
        window_props_.width = windowjs.value("width", 150);
        window_props_.height = windowjs.value("height", 300);
        window_props_.rounding = windowjs.value("rounding", 0.0f);
        window_props_.FloatToImU32();
    }

}