#include "TextWidget.h"
#include <imgui_internal.h>
#include <imgui_stdlib.h>
#include "../managers/RuntimeWidgetFactory.h"

namespace rn {

    REGISTER_WIDGET(TextWidget);

    TextWidget::TextWidget()
    {
        SetWidgetClass("TextWidget");
    }

    TextWidget::TextWidget(const std::string& name, const ImVec2& pos)
        : Widget(name, WidgetType::TEXT, pos, ImVec2(150, 60)) {
        SetWidgetClass("TextWidget");
    }

    void TextWidget::Render(ImDrawList* draw_list) {
        // Вызываем базовую отрисовку (фон, рамка, имя)
        //Widget::Render(draw_list);


        // Отрисовываем текст
        ImVec2 screen_max = GetScreenMax();
        
        DrawTextContent(draw_list, ImVec2(screen_max.x - (size_.x * 0.5f) - (text_.length() * 3), screen_max.y - (size_.y) * 0.5f));
    }

    void TextWidget::DrawTextContent(ImDrawList* draw_list, const ImVec2& screen_pos) {
        // Простая отрисовка текста
        draw_list->AddText(screen_pos, text_color_, text_.c_str());
    }

    void TextWidget::RenderContent(ImVec2& screen_min, ImVec2& screen_max) {
        // Для текстового виджета не нужно ImGui содержимое
        // Всё рисуется через DrawList
    }

   

    void TextWidget::FromJson(const nlohmann::json& json) {
        Widget::FromJson(json);
        text_ = json.value("text", "");
        font_scale_ = json.value("font_scale", 1.0f);
        wrap_text_ = json.value("wrap_text", false);
    }
}