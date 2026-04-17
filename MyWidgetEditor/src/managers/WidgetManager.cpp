#include "WidgetManager.h"
#include "TextWidget.h"
#include "InputTextWidget.h"
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
            if (widget) {
            widget->Render(draw_list, canvas_p0);
            }
        }
    }

    void WidgetManager::RenderContentAll(const ImVec2& canvas_p0) {
        for (auto& widget : widgets_) {
            if (widget) {
            ImVec2 screen_min = widget->GetScreenMin(canvas_p0);
            ImVec2 screen_max = widget->GetScreenMax(canvas_p0); 

            widget->RenderContent(screen_min, screen_max);
            }
        }
    }

    void WidgetManager::SelectWidget(int index)
    {
        selected_widget_index_ = index;
    }

    void WidgetManager::DeselectAll()
    {

    }

    Widget* WidgetManager::GetSelectedWidget() {
        
        for (int i = 0; i < widgets_.size();++i) {
            if (widgets_[i]->IsSelected()) {
                selected_widget_index_ = i;
                return widgets_[i].get();
            }
        }
        return nullptr;
    }

    void WidgetManager::DeleteWidget(const std::string& id) {

        auto it = std::find_if(connections_.begin(), connections_.end(), [id](const Connection& p) {
            return p.from.widget_id == id || p.to.widget_id == id;
            });

        if (it != connections_.end()) { connections_.erase(it); }

        widgets_by_id_.erase(id);
        for (auto it = widgets_.begin(); it != widgets_.end(); ++it) {
            if ((*it)->GetId() == id) {
                widgets_.erase(it);
                break;
            }
        }

    }

    void WidgetManager::SaveToFile(const std::string& filename, const wg::RuntimeWindowProperties& mw_props) const {
        nlohmann::json json = ToJson(mw_props);
        std::ofstream file(filename);
        if (file.is_open()) {
            file << json.dump(4);
        }
    }

    void WidgetManager::LoadFromFile(const std::string& filename, wg::RuntimeWindowProperties& mw_props) {
        std::ifstream file(filename);
        if (file.is_open()) {
            nlohmann::json json;
            file >> json;           
            FromJson(json, mw_props);
        }
    }

    nlohmann::json WidgetManager::ToJson(const wg::RuntimeWindowProperties& mw_props) const {
        nlohmann::json json_array = nlohmann::json::array();
        nlohmann::json window;
        
        window["bg_color"] = nlohmann::json::array({
                mw_props.bg_color_float[0],
                mw_props.bg_color_float[1],
                mw_props.bg_color_float[2],
                mw_props.bg_color_float[3]
            });

        window["full_screen"] = mw_props.full_screen;
        window["always_on_top"] = mw_props.always_on_top;
        window["always_on_bottom"] = mw_props.always_on_bottom;
        window["window_rounding"] = mw_props.window_rounding;
        window["resizeble"] = mw_props.resizeble;
        window["mouse_passthrougth"] = mw_props.mouse_passthrougth;
        window["moveble"] = mw_props.moveble;
        window["decorated"] = mw_props.decorated;

        window["frag_shader"] = mw_props.frag_GLSLshader_file;
        window["width"] = mw_props.width;
        window["height"] = mw_props.height;
        window["rounding"] = mw_props.rounding;

        for (const auto& widget : widgets_) {
            json_array.push_back(widget->ToJson());
        }
                
        return { {"window", window},{"widgets", json_array}};
    }

    void WidgetManager::FromJson(const nlohmann::json& json, wg::RuntimeWindowProperties& mw_props) {
        widgets_.clear();
        widgets_by_id_.clear();
               
        if (!json.contains("widgets") || !json["widgets"].is_array()) {
            return;
        }
        if (!json.contains("window")) {            
            return;
        }

       const nlohmann::json& windowjs = json["window"];

      if (windowjs.contains("bg_color") && windowjs["bg_color"].is_array()) {
          mw_props.bg_color_float[0] = windowjs["bg_color"].at(0).get<float>();
          mw_props.bg_color_float[1] = windowjs["bg_color"].at(1).get<float>();
          mw_props.bg_color_float[2] = windowjs["bg_color"].at(2).get<float>();
          mw_props.bg_color_float[3] = windowjs["bg_color"].at(3).get<float>();
      }
      
      mw_props.always_on_top = windowjs.value("full_screen", false);
      mw_props.always_on_top = windowjs.value("always_on_top",false);
      mw_props.always_on_bottom = windowjs.value("always_on_bottom", false);
      mw_props.resizeble = windowjs.value("resizeble", true);
      mw_props.mouse_passthrougth = windowjs.value("mouse_passthrougth", false);
      mw_props.moveble = windowjs.value("moveble", true);
      mw_props.decorated = windowjs.value("decorated", true);

       mw_props.frag_GLSLshader_file = windowjs.value("frag_shader", "");
       mw_props.width = windowjs.value("width", 150);
       mw_props.height = windowjs.value("height", 300);
       mw_props.rounding = windowjs.value("rounding", 0.0f);
                   
       //Check connections
       if (json.contains("connections") && json["connections"].is_array()) {
           for (const auto& connection : json["connections"])
           {
               connections_.push_back(Connection{
                    PortRef{connection["from_port"],connection["from_widget"]},
                    PortRef{connection["to_port"],connection["to_widget"]}
                   });
           }
       }
       
        for (const auto& widget_json : json["widgets"]) {
            // Пробуем создать виджет через фабрику
            std::unique_ptr<Widget> widget = WidgetFactory::CreateFromJson(widget_json);            
            if (widget) {                           
                // Виджет уже создан и FromJson 
                widget.get()->FromJson(widget_json);
                widgets_by_id_[widget.get()->GetId()] = widget.get();
                widgets_.push_back(std::move(widget));

            }
            else {
                std::cout << "Фабрика не смогла создать (старый формат или не зарегистрирован)";
                
            }
        }
    }

    
}