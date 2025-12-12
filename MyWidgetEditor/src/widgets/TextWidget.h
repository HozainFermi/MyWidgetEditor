#pragma once
#include "Widget.h"
#include <string>

namespace wg {
    class TextWidget : public Widget {
    private:
        std::string text_;
        float font_scale_ = 1.0f;
        bool wrap_text_ = false;

    public:
        TextWidget(const std::string& name, const ImVec2& pos, const std::string& text = "");

        void Render(ImDrawList* draw_list, const ImVec2& canvas_p0) override;
        void RenderContent() override;

        // Специфичные методы
        void SetText(const std::string& text) { text_ = text; }
        const std::string& GetText() const { return text_; }

        nlohmann::json ToJson() const override;
        void FromJson(const nlohmann::json& json) override;

    private:
        void DrawTextContent(ImDrawList* draw_list, const ImVec2& screen_pos);
    };
}