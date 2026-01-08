#include "Widget.h"
#include <imgui_internal.h>
#include <imgui_stdlib.h>

namespace rn {

    int Widget::next_id_ = 0;

    Widget::Widget()
        : name_("Unnamed"), position_(0, 0), size_(100, 50) {
        //static int next_id = 0;
        id_ = "widget_" + std::to_string(next_id_++);
        widget_class_ = "Widget";  // Базовый класс
    }

    Widget::Widget(const std::string& name, WidgetType type, const ImVec2& pos, const ImVec2& size)
        : name_(name), type_(type), position_(pos), size_(size) {
        id_ = "widget_" + std::to_string(next_id_++);
    }

    bool Widget::UpdateInteraction(int widget_id)
    {
        return true;
    }

    void Widget::Render(ImDrawList* draw_list)
    {

    }

    void Widget::RenderContent(ImVec2& screen_min, ImVec2& screen_max)
    {
    }



    void Widget::FromJson(const nlohmann::json& json) {
        if (json.contains("id")) id_ = json["id"];
        if (json.contains("name")) name_ = json["name"];
        if (json.contains("widget_class")) { widget_class_ = json["widget_class"]; }
        if (json.contains("type")) { type_ = static_cast<WidgetType>(json["type"].get<int>()); }
        if (json.contains("position") && json["position"].is_array()) {
            position_.x = json["position"][0];
            position_.y = json["position"][1];
        }
        if (json.contains("size") && json["size"].is_array()) {
            size_.x = json["size"][0];
            size_.y = json["size"][1];
        }
        if (json.contains("bg_color")) bg_color_ = json["bg_color"];
        if (json.contains("min_width")) min_width_ = json["min_width"];
        if (json.contains("min_height")) min_height_ = json["min_height"];
    }


}