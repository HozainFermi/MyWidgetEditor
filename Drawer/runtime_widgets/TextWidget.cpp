#include "TextWidget.h"
#include <imgui_internal.h>
#include <imgui_stdlib.h>

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
        ImVec2 screen_min = GetScreenMin();
        
        DrawTextContent(draw_list, screen_min, screen_max);
    }

    void TextWidget::DrawTextContent(ImDrawList* draw_list, const ImVec2& screen_min, const ImVec2& screen_max) {        
        ImVec2 text_begin_screen_pos{};
        text_begin_screen_pos.y = screen_min.y + 20;
        float wrap_width = size_.x - 20;
        const char* text_end = text_.c_str() + text_.length();

        ImFont* font = ImGui::GetFont();
        ImVec2 text_size = font->CalcTextSizeA(font->Scale * 13, wrap_width, 0.0f, text_.c_str(), text_end);
        if (left_alignment_) {
            text_begin_screen_pos.x = screen_min.x + 5;
        }
        if (center_alignment_) {
            text_begin_screen_pos.x = screen_max.x - (size_.x * 0.5f) - (text_size.x / 2);
        }
        if (right_alignment_) {
            text_begin_screen_pos.x = screen_max.x - (size_.x * 0.5f);
        }

        for (auto segment : parse_result_) {            
            AddTextWithEffect(draw_list, text_begin_screen_pos, wrap_width, segment);
        }

        if (text_size.y >= size_.y) { size_.y = text_size.y + 10; }

    }

    void TextWidget::AddTextWithEffect(ImDrawList* draw_list, ImVec2& text_begin_screen_pos, const float& wrap_width, const TextSegment& segment)
    {
        if (parsing_error_) {
            draw_list->AddText(text_begin_screen_pos,text_color_,error_message_,error_message_+19);
            return;
        }
        ImFont* font = ImGui::GetFont();
        float font_size = 13 * font_scale_;
        float start_x = text_begin_screen_pos.x;

        const char* text_ptr = segment.content.c_str();
        const char* text_end = text_ptr + segment.content.length();
        int char_idx = 0;


        static ImU32 rainbow_colors[7] = {
            IM_COL32(255, 0, 0, 255), IM_COL32(255, 127, 0, 255), IM_COL32(255, 255, 0, 255),
            IM_COL32(0, 255, 0, 255), IM_COL32(0, 0, 255, 255), IM_COL32(75, 0, 130, 255), IM_COL32(148, 0, 211, 255)
        };

        while (text_ptr < text_end) {

            unsigned int c;
            const char* next_ptr = text_ptr + ImTextCharFromUtf8(&c, text_ptr, text_end);


            float char_width = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, text_ptr, next_ptr).x;

            if (wrap_width > 0.0f && (text_begin_screen_pos.x + char_width) > (start_x + wrap_width)) {
                text_begin_screen_pos.x = start_x;
                text_begin_screen_pos.y += font_size; // Переход на следующую строку
            }

            ImU32 col = text_color_;
            ImVec2 draw_pos = text_begin_screen_pos;

            if (segment.flags == 0) {
            }
            if (segment.flags & TextEffects_::TextEffect_Wave) {
                float wave = sinf((float)ImGui::GetTime() * 6.f + char_idx * 0.5f) * 3.f;
                draw_pos.y += wave;
            }
            if (segment.flags & TextEffects_::TextEffect_Rainbow) {
                float time = (float)ImGui::GetTime() * 5.f;
                col = rainbow_colors[(char_idx + (int)time) % 7];
            }

            draw_list->AddText(font, font_size, draw_pos, col, text_ptr, next_ptr);

            text_begin_screen_pos.x += char_width;

            text_ptr = next_ptr;
            char_idx++;
        }
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

        if (json.value("left_alignment", false)) { left_alignment_ = true; }
        if (json.value("center_alignment", false)) { center_alignment_ = true; }
        if (json.value("right_alignment", false)) { right_alignment_ = true; }

        try {
            parse_result_ = std::move(BBCodeParser::Parse(text_));
            parsing_error_ = false;
        }
        catch (const std::invalid_argument& ex) {
            std::cerr << ex.what() << std::endl;
            parsing_error_ = true;
            error_message_ = ex.what();
        }
    }
}