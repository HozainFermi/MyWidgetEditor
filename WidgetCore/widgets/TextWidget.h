#pragma once
#include <string>
#include <vector>
#include <BBCodeParser.h>
#include "core/Widget.h"

namespace core {

    // Текст с BBCode: [b], [i], [wave], [rainbow], [shake]
    class TextWidget : public Widget {
    public:
        TextWidget();

        void Render(ImDrawList* draw_list, const ImVec2& min, const ImVec2& max) override;
        void RenderProperties() override;
        float GetDesiredFps() const override;

        void SetText(const std::string& text) { text_ = text; Reparse(); }
        const std::string& GetText() const { return text_; }

        nlohmann::json ToJson() const override;
        void FromJson(const nlohmann::json& json) override;

    private:
        void Reparse();
        void AddTextWithEffect(ImDrawList* draw_list, ImVec2& pen, float line_start_x, float wrap_width, const TextSegment& segment, int& char_idx);

        std::string text_ = "Sample text";
        std::vector<TextSegment> parse_result_;
        float font_scale_ = 1.0f;
        bool wrap_text_ = true;

        bool parsing_error_ = false;
        std::string error_message_;

        bool left_alignment_ = false;
        bool center_alignment_ = true;
        bool right_alignment_ = false;
    };
}
