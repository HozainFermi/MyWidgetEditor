#include "InputTextWidget.h"
#include <imgui_internal.h>
#include <imgui_stdlib.h>
#include <string>

namespace rn {

    REGISTER_WIDGET(InputTextWidget);

    InputTextWidget::InputTextWidget()
    {
        SetWidgetClass("InputTextWidget");
    }

    InputTextWidget::InputTextWidget(const std::string& name, const ImVec2& pos)
        : Widget(name, WidgetType::INPUT_TEXT, pos, ImVec2(200, 80)) {
        label_ = name + ":";
        buffer_.resize(max_length_);

        SetWidgetClass("InputTextWidget");
    }

    bool InputTextWidget::UpdateInteraction(int widget_id) {

        // Вызываем базовую логику для остальных случаев
        return Widget::UpdateInteraction(widget_id);
    }

    void InputTextWidget::Render(ImDrawList* draw_list) {
        // Вызываем базовую отрисовку (фон, рамка, имя)
        Widget::Render(draw_list);

    }

    void InputTextWidget::RenderContent(ImVec2& screen_min, ImVec2& screen_max) {

        ImVec2 size = ImVec2(screen_max.x - screen_min.x,
            screen_max.y - screen_min.y);

        ImVec2 text_input_size = ImVec2(size.x * 0.8f,  // 80% ширины виджета
            size.y * 0.80f); // 80% высоты

        // Центрируем
        ImVec2 input_pos = ImVec2(
            screen_min.x + (size.x - text_input_size.x) * 0.5f,
            screen_min.y + (size.y - text_input_size.y) * 1.5f
        );

        ImGui::SetCursorScreenPos(input_pos);
        ImGui::PushID(this);
        ImGui::BeginChild("##input_child", text_input_size, false,
            ImGuiWindowFlags_NoDecoration);
        //|ImGuiWindowFlags_NoScrollbar

        // Центрируем текст внутри InputText
        ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2(5, 10));

        if (ImGui::InputTextMultiline("##input", &buffer_,
            ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 3))) {
            OnValueChanged();
        }
        ImGui::PopStyleVar();
        ImGui::EndChild();
        ImGui::PopID();
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