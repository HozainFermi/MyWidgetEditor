#include "InputTextWidget.h"
#include <imgui_internal.h>
#include <imgui_stdlib.h>

namespace wg {

    ImVec2 can_p0;

    InputTextWidget::InputTextWidget(const std::string& name, const ImVec2& pos)
        : Widget(name, WidgetType::INPUT_TEXT, pos, ImVec2(200, 80)) {
        label_ = name + ":";
        buffer_.resize(max_length_);
        bg_color_ = IM_COL32(30, 30, 60, 255);
    }

    bool InputTextWidget::UpdateInteraction(const ImVec2& canvas_p0, int widget_id) {
        can_p0 = canvas_p0;
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 mouse_pos = io.MousePos;

        ImRect screen_rect = GetScreenRect(canvas_p0);
        ImRect header_rect = ImRect(
            screen_rect.Min.x, screen_rect.Min.y,
            screen_rect.Max.x, screen_rect.Min.y + 20
        );

        // Проверяем клик на заголовке (для перетаскивания)
        bool is_header_hovered = header_rect.Contains(mouse_pos);

        // InputText может заблокировать перетаскивание, если клик был на текстовом поле
        bool block_drag = ShouldBlockDrag();

        if (!block_drag && is_header_hovered && ImGui::IsMouseClicked(0)) {
            // Перетаскивание за заголовок
            is_dragging_ = true;
            drag_offset_ = ImVec2(
                mouse_pos.x - screen_rect.Min.x,
                mouse_pos.y - screen_rect.Min.y
            );
            return true;
        }

        // Вызываем базовую логику для остальных случаев
        return Widget::UpdateInteraction(canvas_p0, widget_id);
    }

    void InputTextWidget::Render(ImDrawList* draw_list, const ImVec2& canvas_p0) {
        // Вызываем базовую отрисовку (фон, рамка, имя)
        Widget::Render(draw_list, canvas_p0);
        ImVec2 screen_min =GetScreenMin(canvas_p0);

        // Фон
        //draw_list->AddRectFilled(screen_min, GetScreenMax(canvas_p0), bg_color_);

        // Заголовок для перетаскивания
        DrawHeader(draw_list, screen_min);

        // Рамка
       //draw_list->AddRect(screen_min, GetScreenMax(canvas_p0),
       //    is_selected_ ? IM_COL32(255, 200, 0, 255) : border_color_,
       //    0.0f, 0, border_thickness_);
    }

    void InputTextWidget::RenderContent() {
        ImVec2 screen_min = GetScreenMin(can_p0); // кастыль какой то

        // Создаём область для ImGui элементов
        ImGui::SetCursorScreenPos(ImVec2(screen_min.x + 5, screen_min.y + 25));

       ImGui::BeginChild(GetId().c_str(),
           ImVec2(size_.x - 10, size_.y - 30),
           false,
           ImGuiWindowFlags_NoDecoration |
           ImGuiWindowFlags_NoMove |
           ImGuiWindowFlags_NoSavedSettings);

        // InputText с поддержкой std::string
        if (is_multiline_) {
            if (ImGui::InputTextMultiline("##input", &buffer_,
                ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 4))) {
                OnValueChanged();
            }
        }
        else {
            if (ImGui::InputText("##input", &buffer_)) {
                OnValueChanged();
            }
        }

        // Подсказка
        if (buffer_.empty() && !hint_.empty()) {
            ImGui::PushStyleColor(ImGuiCol_Text, IM_COL32(100, 100, 100, 255));
            ImGui::SetCursorPos(ImVec2(4, 4));
            ImGui::Text("%s", hint_.c_str());
            ImGui::PopStyleColor();
        }

        ImGui::EndChild();
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