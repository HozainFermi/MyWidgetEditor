#pragma once
#include "Widget.h"
#include <string>
#include "WidgetFactory.h"
#include <BBCodeParser.h>

namespace wg {
    class TextWidget : public Widget {
    private:
        std::string text_="Sample text";
        std::vector<TextSegment> parse_result_;
        float font_scale_ = 1.0f;
        
        bool parsing_error_ = false;
        const char* error_message_;

        bool wrap_text_ = false;
        bool left_alignment_ = false;
        bool center_alignment_ = true;
        bool right_alignment_ = false;


    public:
        TextWidget();
        TextWidget(const std::string& name, const ImVec2& pos);

        void Render(ImDrawList* draw_list, const ImVec2& canvas_p0) override;
        void RenderContent(ImVec2& screen_min, ImVec2& screen_max) override;
        void RenderProperties() override;

        // Специфичные методы
        void SetText(const std::string& text) { text_ = text; }
        const std::string& GetText() const { return text_; }

        nlohmann::json ToJson() const override;
        void FromJson(const nlohmann::json& json) override;

    private:
        void DrawTextContent(ImDrawList* draw_list, const ImVec2& screen_min, const ImVec2& screen_max);
        void AddTextWithEffect(ImDrawList* draw_list, ImVec2& text_begin_screen_pos, const float& wrap_width, const TextSegment& segment);
        void ShowParsingError(const char* what);
    };
}