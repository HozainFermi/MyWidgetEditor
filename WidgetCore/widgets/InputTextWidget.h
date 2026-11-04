#pragma once
#include <string>
#include "core/Widget.h"

namespace core {

    class InputTextWidget : public Widget {
    public:
        InputTextWidget();

        void RenderContent(const ImVec2& min, const ImVec2& max) override;

        void SetValue(const std::string& value) { buffer_ = value; }
        const std::string& GetValue() const { return buffer_; }

        nlohmann::json ToJson() const override;
        void FromJson(const nlohmann::json& json) override;

    private:
        std::string buffer_;
        std::string label_;
        std::string hint_;
        int max_length_ = 256;
        bool is_multiline_ = false;
    };
}
