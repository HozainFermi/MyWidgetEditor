#include "Widget.h"
#include <imgui_internal.h>
#include <imgui_stdlib.h>

namespace rn {

    int Widget::next_id_ = 0;

    Widget::Widget()
        : name_("Unnamed"), position_(0, 0), size_(100, 50) {
        static int next_id = 0;
        id_ = "widget_" + std::to_string(next_id++);
        widget_class_ = "Widget";  // Базовый класс
    }

    Widget::Widget(const std::string& name, WidgetType type, const ImVec2& pos, const ImVec2& size)
        : name_(name), type_(type), position_(pos), size_(size) {
        id_ = "widget_" + std::to_string(next_id_++);
    }

    bool Widget::UpdateInteraction(int widget_id)
    {
        return false;
    }

    void Widget::Render(ImDrawList* draw_list)
    {

    }

    void Widget::RenderContent(ImVec2& screen_min, ImVec2& screen_max)
    {
    }



}