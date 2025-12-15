#include "TextWidget.h"
#include <imgui_internal.h>
#include <imgui_stdlib.h>

namespace wg {
    TextWidget::TextWidget(const std::string& name, const ImVec2& pos, const std::string& text)
        : Widget(name, WidgetType::TEXT, pos, ImVec2(150, 60)), text_(text) {
        bg_color_ = IM_COL32(40, 40, 80, 255);
    }

    void TextWidget::Render(ImDrawList* draw_list, const ImVec2& canvas_p0) {
        // Вызываем базовую отрисовку (фон, рамка, имя)
        Widget::Render(draw_list, canvas_p0);
        

        // Отрисовываем текст
        ImVec2 screen_max = GetScreenMax(canvas_p0);
        //screen_max.x*0.5f-text_.length()*0.1f*0.5f, screen_max.y*0.5f
        DrawTextContent(draw_list, ImVec2(screen_max.x-(size_.x*0.5f)-(text_.length()*3), screen_max.y - (size_.y) * 0.5f));
    }

    void TextWidget::DrawTextContent(ImDrawList* draw_list, const ImVec2& screen_pos) {
        // Простая отрисовка текста
        draw_list->AddText(screen_pos, text_color_, text_.c_str());

        // Если нужно переносить текст:
        
       /* ImFont* font = ImGui::GetFont();
        float wrap_width = size_.x - 20;
        const char* text_end = text_.c_str() + text_.length();
        ImVec2 text_size = font->CalcTextSizeA(font->FontSize, wrap_width, 0.0f, text_.c_str(), text_end);

        draw_list->AddText(font, font->FontSize * font_scale_, screen_pos,
                          text_color_, text_.c_str(), text_end, wrap_width);
        */
    }

    void TextWidget::RenderContent() {
        // Для текстового виджета не нужно ImGui содержимое
        // Всё рисуется через DrawList
    }

    nlohmann::json TextWidget::ToJson() const {
        auto json = Widget::ToJson();
        json["text"] = text_;
        json["font_scale"] = font_scale_;
        json["wrap_text"] = wrap_text_;
        return json;
    }

    void TextWidget::FromJson(const nlohmann::json& json) {
        Widget::FromJson(json);
        text_ = json.value("text", "");
        font_scale_ = json.value("font_scale", 1.0f);
        wrap_text_ = json.value("wrap_text", false);
    }
}