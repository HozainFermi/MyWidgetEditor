#pragma once
#include "Widget.h"
#include "../managers/RuntimeWidgetFactory.h"
#include <string>

namespace rn {
    class InputTextWidget : public Widget {
    private:
        std::string buffer_;
        std::string label_;
        std::string hint_;
        int max_length_ = 256;
        bool is_multiline_ = false;
        bool is_password_ = false;

    public:
        InputTextWidget();
        InputTextWidget(const std::string& name, const ImVec2& pos);

        bool UpdateInteraction(int widget_id) override;
        void Render(ImDrawList* draw_list) override;
        void RenderContent(ImVec2& screen_min, ImVec2& screen_max) override;

        void SetValue(const std::string& value) { buffer_ = value; }
        const std::string& GetValue() const { return buffer_; }

        nlohmann::json ToJson() const;
        void FromJson(const nlohmann::json& json);

    private:

    };
}