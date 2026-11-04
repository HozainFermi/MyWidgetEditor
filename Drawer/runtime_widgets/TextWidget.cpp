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
        const float font_size = 13.0f * font_scale_;
        const float wrap_width = size_.x - 20;
        ImFont* font = ImGui::GetFont();

        if (parsing_error_) {
            draw_list->AddText(font, font_size, ImVec2(screen_min.x + 5, screen_min.y + 20),
                IM_COL32(255, 90, 90, 255), error_message_.c_str(), nullptr, wrap_width);
            return;
        }

        // Размер считаем по тексту без тегов, иначе центрирование уезжает
        std::string plain_text;
        for (const auto& segment : parse_result_) { plain_text += segment.content; }
        ImVec2 text_size = font->CalcTextSizeA(font_size, FLT_MAX, wrap_width,
            plain_text.c_str(), plain_text.c_str() + plain_text.size());
        float block_width = ImMin(text_size.x, wrap_width);

        float line_start_x = screen_min.x + 5;
        if (center_alignment_) { line_start_x = screen_min.x + (size_.x - block_width) * 0.5f; }
        if (right_alignment_)  { line_start_x = screen_max.x - 5 - block_width; }

        ImVec2 pen(line_start_x, screen_min.y + 20);
        int char_idx = 0;
        for (const auto& segment : parse_result_) {
            AddTextWithEffect(draw_list, pen, line_start_x, wrap_width, segment, char_idx);
        }

        if (text_size.y >= size_.y) { size_.y = text_size.y + 10; }
    }

    void TextWidget::AddTextWithEffect(ImDrawList* draw_list, ImVec2& pen, float line_start_x, float wrap_width, const TextSegment& segment, int& char_idx)
    {
        ImFont* font = ImGui::GetFont();
        const float font_size = 13.0f * font_scale_;
        const float time = (float)ImGui::GetTime();
        const bool bold = segment.flags & TextEffect_Bold;
        const bool italic = segment.flags & TextEffect_Italic;

        static const ImU32 rainbow_colors[7] = {
            IM_COL32(255, 0, 0, 255), IM_COL32(255, 127, 0, 255), IM_COL32(255, 255, 0, 255),
            IM_COL32(0, 255, 0, 255), IM_COL32(0, 0, 255, 255), IM_COL32(75, 0, 130, 255), IM_COL32(148, 0, 211, 255)
        };

        const char* text_ptr = segment.content.c_str();
        const char* text_end = text_ptr + segment.content.length();

        while (text_ptr < text_end) {
            unsigned int c;
            const char* next_ptr = text_ptr + ImTextCharFromUtf8(&c, text_ptr, text_end);

            if (c == '\n') {
                pen.x = line_start_x;
                pen.y += font_size;
                text_ptr = next_ptr;
                continue;
            }

            float char_width = font->CalcTextSizeA(font_size, FLT_MAX, 0.0f, text_ptr, next_ptr).x;
            if (bold) { char_width += 1.0f; }

            // Перенос считаем от начала строки, а не от начала сегмента
            if (wrap_width > 0.0f && pen.x > line_start_x && (pen.x + char_width) > (line_start_x + wrap_width)) {
                pen.x = line_start_x;
                pen.y += font_size;
            }

            ImU32 col = text_color_;
            ImVec2 draw_pos = pen;

            if (segment.flags & TextEffect_Wave) {
                draw_pos.y += sinf(time * 6.f + char_idx * 0.5f) * 3.f;
            }
            if (segment.flags & TextEffect_Shake) {
                draw_pos.x += sinf(time * 40.f + char_idx * 12.9898f) * 1.5f;
                draw_pos.y += cosf(time * 37.f + char_idx * 78.233f) * 1.5f;
            }
            if (segment.flags & TextEffect_Rainbow) {
                col = rainbow_colors[(char_idx + (int)(time * 5.f)) % 7];
            }

            int vtx_start = draw_list->VtxBuffer.Size;
            draw_list->AddText(font, font_size, draw_pos, col, text_ptr, next_ptr);
            if (bold) {
                // Псевдо-жирный: рисуем символ второй раз со сдвигом на пиксель
                draw_list->AddText(font, font_size, ImVec2(draw_pos.x + 1.0f, draw_pos.y), col, text_ptr, next_ptr);
            }
            if (italic) {
                // Псевдо-курсив: наклоняем вершины символа относительно базовой линии
                float baseline_y = draw_pos.y + font_size;
                for (int v = vtx_start; v < draw_list->VtxBuffer.Size; ++v) {
                    ImDrawVert& vert = draw_list->VtxBuffer[v];
                    vert.pos.x += (baseline_y - vert.pos.y) * 0.2f;
                }
            }

            pen.x += char_width;
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

        left_alignment_ = json.value("left_alignment", false);
        center_alignment_ = json.value("center_alignment", false);
        right_alignment_ = json.value("right_alignment", false);
        if (!left_alignment_ && !right_alignment_) { center_alignment_ = true; }

        Reparse();
    }

    void TextWidget::Reparse() {
        try {
            parse_result_ = BBCodeParser::Parse(text_);
            parsing_error_ = false;
            error_message_.clear();
        }
        catch (const std::invalid_argument& ex) {
            std::cerr << ex.what() << std::endl;
            parse_result_.clear();
            parsing_error_ = true;
            error_message_ = ex.what();
        }
    }
}