#include "TextWidget.h"
#include <imgui_internal.h>
#include <imgui_stdlib.h>
#include <IconText.h>

namespace wg {

    REGISTER_WIDGET(TextWidget);

    TextWidget::TextWidget()
    {
        SetWidgetClass("TextWidget");
    }

    TextWidget::TextWidget(const std::string& name, const ImVec2& pos)
        : Widget(name, WidgetType::TEXT, pos, ImVec2(150, 60)) {
        SetWidgetClass("TextWidget");
    }

    void TextWidget::Render(ImDrawList* draw_list, const ImVec2& canvas_p0) {
        // Вызываем базовую отрисовку (фон, рамка, имя)
        Widget::Render(draw_list, canvas_p0);
        

        // Отрисовываем текст
        ImVec2 screen_max = GetScreenMax(canvas_p0);
        ImVec2 screen_min = GetScreenMin(canvas_p0);
        //screen_max.x*0.5f-text_.length()*0.1f*0.5f, screen_max.y*0.5f
        //screen_max.x-(size_.x*0.5f)-(text_.length()*3), screen_max.y - (size_.y) * 0.5f)
        DrawTextContent(draw_list, screen_min, screen_max);
    }

    void TextWidget::DrawTextContent(ImDrawList* draw_list, const ImVec2& screen_min, const ImVec2& screen_max) {

        ImVec2 text_begin_screen_pos{};
        text_begin_screen_pos.y = screen_min.y + 20;

       ImFont* font = ImGui::GetFont();
       float wrap_width = size_.x - 20;
       const char* text_end = text_.c_str() + text_.length();
       ImVec2 text_size = font->CalcTextSizeA(font->Scale*13, wrap_width, 0.0f, text_.c_str(), text_end);
       
        if (left_alignment_) {
            text_begin_screen_pos.x = screen_min.x + 5;            
            draw_list->AddText(font, font->Scale * 13 * font_scale_, text_begin_screen_pos,
                text_color_, text_.c_str(), text_end, wrap_width);            
        }

        if (center_alignment_) {            
            text_begin_screen_pos.x = screen_max.x - (size_.x * 0.5f) - (text_size.x/2);            
            draw_list->AddText(font, font->Scale * 13 * font_scale_, text_begin_screen_pos,
                text_color_, text_.c_str(), text_end, wrap_width);
        }

        if (right_alignment_) {

        }
        std::cout << "TEXT-SIZE Y:" << text_size.y << std::endl;
        std::cout << "SIZE Y:" << size_.y << std::endl;

        if (text_size.y >= size_.y) { size_.y = text_size.y + 10; }
              
    }



    void TextWidget::RenderContent(ImVec2& screen_min, ImVec2& screen_max) {
        // Для текстового виджета не нужно ImGui содержимое
        // Всё рисуется через DrawList
    }
    

    void TextWidget::RenderProperties()
    {
        auto flags = ImGuiInputTextFlags_NoHorizontalScroll | ImGuiInputTextFlags_NoUndoRedo;
        ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 5.0f);
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.16f, 0.16f, 0.16f, 1.0f));
        ImVec2 btn_size{ 25,20 };
        ImGui::SetWindowFontScale(1.5f);

        ImGui::PushID("btn-align");
        if (ImGui::Button(ICON(ICON_ALIGN_LEFT), btn_size)) { left_alignment_ = true;  center_alignment_ = false; right_alignment_ = false;}  
        ImGui::SameLine();
        if (ImGui::Button(ICON(ICON_ALIGN_CENTER), btn_size)) { left_alignment_ = false;  center_alignment_ = true; right_alignment_ = false; } 
        ImGui::SameLine();
        if (ImGui::Button(ICON(ICON_ALIGN_RIGHT), btn_size)) { left_alignment_ = false;  center_alignment_ = false; right_alignment_ = true; }
        ImGui::PopID();
        ImGui::PopStyleVar();
        ImGui::PopStyleColor(1);
        ImGui::SetWindowFontScale(1.0f);

        ImGui::InputText("##textwContent", &text_, flags);
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            
        }
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