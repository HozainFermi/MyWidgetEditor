#include "InputTextWidget.h"
#include <imgui_internal.h>
#include <imgui_stdlib.h>
#include <string>

namespace wg {

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

    bool InputTextWidget::UpdateInteraction(const ImVec2& canvas_p0, const ImVec2& canvas_size, int widget_id) {
        
        // Вызываем базовую логику для остальных случаев
        return Widget::UpdateInteraction(canvas_p0, canvas_size, widget_id);
    }

    void InputTextWidget::Render(ImDrawList* draw_list, const ImVec2& canvas_p0) {
        // Вызываем базовую отрисовку (фон, рамка, имя)
        Widget::Render(draw_list, canvas_p0);
        
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
            ImGuiWindowFlags_NoDecoration );
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

    void InputTextWidget::DrawHeader(ImDrawList* draw_list, const ImVec2& screen_min) {
        ImVec2 header_max = ImVec2(screen_min.x + size_.x-2, screen_min.y + 20);
        ImVec2 header_min = ImVec2(screen_min.x + 2, screen_min.y + 2);
        // Фон заголовка
        draw_list->AddRectFilled(header_min, header_max, IM_COL32(60, 60, 100, 255));

        // Текст заголовка
        draw_list->AddText(ImVec2(screen_min.x + 5, screen_min.y + 2),
            text_color_, label_.c_str());

        // Индикатор перетаскивания
        if (is_hovered_) {
            draw_list->AddText(ImVec2(header_max.x - 45, screen_min.y + 2),
                IM_COL32(200, 200, 200, 255), "? drag");
        }
    }

    bool InputTextWidget::ShouldBlockDrag() const {
        // Не перетаскиваем если InputText активен или ховерен
        return ImGui::IsItemActive() || ImGui::IsItemHovered();
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