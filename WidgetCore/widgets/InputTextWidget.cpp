#include "InputTextWidget.h"
#include <imgui_internal.h>
#include <imgui_stdlib.h>

namespace core {

    InputTextWidget::InputTextWidget()
        : Widget(WidgetType::INPUT_TEXT, ImVec2(200, 80)) {
    }

    void InputTextWidget::RenderContent(const ImVec2& screen_min, const ImVec2& screen_max) {
        ImVec2 size(screen_max.x - screen_min.x, screen_max.y - screen_min.y);
        ImVec2 input_size(size.x * 0.8f, size.y * 0.8f);

        ImVec2 input_pos(
            screen_min.x + (size.x - input_size.x) * 0.5f,
            screen_min.y + (size.y - input_size.y) * 1.5f);

        ImGui::SetCursorScreenPos(input_pos);
        ImGui::PushID(this);
        ImGui::BeginChild("##input_child", input_size, false, ImGuiWindowFlags_NoDecoration);
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 10));
        ImGui::InputTextMultiline("##input", &buffer_, ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 3));
        ImGui::PopStyleVar();
        ImGui::EndChild();
        ImGui::PopID();
    }

    nlohmann::json InputTextWidget::ToJson() const {
        auto json = Widget::ToJson();
        json["buffer"] = buffer_;
        json["label"] = label_;
        json["hint"] = hint_;
        json["max_length"] = max_length_;
        json["is_multiline"] = is_multiline_;
        return json;
    }

    void InputTextWidget::FromJson(const nlohmann::json& json) {
        Widget::FromJson(json);
        buffer_ = json.value("buffer", "");
        label_ = json.value("label", name_ + ":");
        hint_ = json.value("hint", "");
        max_length_ = json.value("max_length", 256);
        is_multiline_ = json.value("is_multiline", false);
    }
}
