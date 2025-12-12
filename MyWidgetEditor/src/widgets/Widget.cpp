#include "Widget.h"
#include <imgui_internal.h>
#include <imgui_stdlib.h>

namespace wg {
    int Widget::next_id_ = 0;

    Widget::Widget(const std::string& name, WidgetType type, const ImVec2& pos, const ImVec2& size)
        : name_(name), type_(type), position_(pos), size_(size) {
        id_ = "widget_" + std::to_string(next_id_++);
    }

    ImVec2 Widget::GetScreenMin(const ImVec2& canvas_p0) const {
        return ImVec2(canvas_p0.x + position_.x, canvas_p0.y + position_.y);
    }

    ImVec2 Widget::GetScreenMax(const ImVec2& canvas_p0) const {
        return ImVec2(canvas_p0.x + position_.x + size_.x,
            canvas_p0.y + position_.y + size_.y);
    }

    bool Widget::ContainsPoint(const ImVec2& point, const ImVec2& canvas_p0) const {
        ImVec2 min = GetScreenMin(canvas_p0);
        ImVec2 max = GetScreenMax(canvas_p0);
        return point.x >= min.x && point.x <= max.x &&
            point.y >= min.y && point.y <= max.y;
    }

    void Widget::SetSelected(bool selected) {
        if (selected != is_selected_) {
            is_selected_ = selected;
            if (selected) {
                OnSelected();
            }
            else {
                OnDeselected();
            }
        }
    }

    bool Widget::UpdateInteraction(const ImVec2& canvas_p0, int widget_id) {
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 mouse_pos = io.MousePos;
        ImRect screen_rect = GetScreenRect(canvas_p0);

        bool changed = false;

        // 1. Всегда обновляем состояние ховера
        is_hovered_ = screen_rect.Contains(mouse_pos);

        // 2. Проверяем ховер над ручками (только если выделен)
        ResizeHandle hovered_handle = ResizeHandle::NONE;
        if (is_selected_) {
            hovered_handle = GetHoveredHandle(mouse_pos, screen_rect);
        }

        // 3. Обрабатываем клик (если не заняты другие элементы)
        if (ImGui::IsMouseClicked(0) && !ImGui::IsAnyItemActive()) {
            if (hovered_handle != ResizeHandle::NONE) {
                // Начинаем ресайз
                is_resizing_ = true;
                active_handle_ = hovered_handle;
                resize_start_size_ = size_;
                resize_start_pos_ = position_;
                changed = true;
            }
            else if (is_hovered_) {
                // Начинаем перетаскивание
                is_dragging_ = true;
                drag_offset_ = ImVec2(
                    mouse_pos.x - screen_rect.Min.x,
                    mouse_pos.y - screen_rect.Min.y
                );
                SetSelected(true);
                changed = true;
            }
            else if (is_selected_) {
                // Клик вне виджета - снимаем выделение
                SetSelected(false);
                changed = true;
            }
        }

        // 4. Обрабатываем ПРОДОЛЖЕНИЕ операций        
        if (is_dragging_ && ImGui::IsMouseDragging(0)) {
            position_ = ImVec2(
                mouse_pos.x - canvas_p0.x - drag_offset_.x,
                mouse_pos.y - canvas_p0.y - drag_offset_.y
            );
            changed = true;
        }

        if (is_resizing_ && ImGui::IsMouseDragging(0)) {
            if (ProcessResizing(canvas_p0, mouse_pos, io.KeyShift)) {
                changed = true;
            }
        }

        // 5. Завершаем операции при отпускании мыши
        if (ImGui::IsMouseReleased(0)) {
            is_dragging_ = false;
            is_resizing_ = false;
            active_handle_ = ResizeHandle::NONE;
            
        }

        return changed;
    }

    void Widget::Render(ImDrawList* draw_list, const ImVec2& canvas_p0) {
        ImRect rect = GetScreenRect(canvas_p0);

        // Фон
        draw_list->AddRectFilled(rect.Min, rect.Max, bg_color_);

        // Общие элементы (рамка, ручки)
        DrawCommonElements(draw_list, rect);

        // Имя виджета в углу
        ImVec2 text_pos = ImVec2(rect.Min.x + 5, rect.Min.y + 5);
        draw_list->AddText(text_pos, text_color_, name_.c_str());
    }

    void Widget::RenderContent() {
        // Базовый виджет не имеет содержимого
        // Переопределяется в потомках
    }

    // ... остальные методы
    void Widget::DrawCommonElements(ImDrawList* draw_list, const ImRect& rect) {
        if (IsSelected()) {
            draw_list->AddRect(rect.Min, rect.Max, selected_border_color_, 0.0f, 0, 2.0f);
        }
        else {
            draw_list->AddRect(rect.Min, rect.Max, border_color_, 0.0f, 0, 2.0f);
        }
        

        // Рисуем ручки ресайза
        const float handle_size = 4.0f;
        ImVec2 handles[8] = {
            rect.Min,                                                 // TOP_LEFT
            ImVec2((rect.Min.x + rect.Max.x) * 0.5f, rect.Min.y),     // TOP
            ImVec2(rect.Max.x, rect.Min.y),                           // TOP_RIGHT
            ImVec2(rect.Min.x, (rect.Min.y + rect.Max.y) * 0.5f),     // LEFT
            ImVec2(rect.Max.x, (rect.Min.y + rect.Max.y) * 0.5f),     // RIGHT
            ImVec2(rect.Min.x, rect.Max.y),                           // BOTTOM_LEFT
            ImVec2((rect.Min.x + rect.Max.x) * 0.5f, rect.Max.y),     // BOTTOM
            rect.Max                                                  // BOTTOM_RIGHT
        };

        if (IsSelected()) {
            for (int i = 0; i < 8; i++) {
                draw_list->AddRectFilled(
                    ImVec2(handles[i].x - handle_size, handles[i].y - handle_size),
                    ImVec2(handles[i].x + handle_size, handles[i].y + handle_size),
                    selected_border_color_
                );
            }
        }
        else {
            for (int i = 0; i < 8; i++) {
                draw_list->AddRectFilled(
                    ImVec2(handles[i].x - handle_size, handles[i].y - handle_size),
                    ImVec2(handles[i].x + handle_size, handles[i].y + handle_size),
                    border_color_
                );
            }
        }
    }

    
    ResizeHandle Widget::GetHoveredHandle(const ImVec2& mouse_pos, const ImRect& rect) const {
        const float handle_size = 4.0f;
        ImVec2 handles[8] = {
            rect.Min,                                                 // TOP_LEFT
            ImVec2((rect.Min.x + rect.Max.x) * 0.5f, rect.Min.y),     // TOP
            ImVec2(rect.Max.x, rect.Min.y),                           // TOP_RIGHT
            ImVec2(rect.Min.x, (rect.Min.y + rect.Max.y) * 0.5f),     // LEFT
            ImVec2(rect.Max.x, (rect.Min.y + rect.Max.y) * 0.5f),     // RIGHT
            ImVec2(rect.Min.x, rect.Max.y),                           // BOTTOM_LEFT
            ImVec2((rect.Min.x + rect.Max.x) * 0.5f, rect.Max.y),     // BOTTOM
            rect.Max                                                  // BOTTOM_RIGHT
        };

        for (int i = 0; i < 8; i++) {
            ImVec2 handle_pos = handles[i];
            ImRect handle_rect(handle_pos.x - handle_size, handle_pos.y - handle_size,
                handle_pos.x + handle_size, handle_pos.y + handle_size);

            if (handle_rect.Contains(mouse_pos)) {
                return static_cast<ResizeHandle>(i + 1);  // Возвращаем ручку
            }
        }

        return ResizeHandle::NONE;  // Важно: всегда возвращаем значение!
    }

    bool Widget::ProcessResizing(const ImVec2& canvas_p0, const ImVec2& mouse_pos, bool proportional) {

        ImVec2 delta = ImGui::GetMouseDragDelta(0);
        ImVec2 old_size = size_;

        // Пропорциональное масштабирование при зажатом Shift
        
        switch (active_handle_) {
        case ResizeHandle(1):
            size_.x -= delta.x;
            size_.y -= proportional ? delta.x : delta.y;
            break;
        case ResizeHandle(2):
            size_.y -= delta.y;
            if (proportional) size_.x = size_.y * (resize_start_size_.x / resize_start_size_.y);
            break;
        case ResizeHandle(3):
            size_.x += delta.x;
            size_.y += proportional ? delta.x : -delta.y;
            break;
        case ResizeHandle(4):
            size_.x -= delta.x;
            if (proportional) size_.y = size_.x * (resize_start_size_.y / resize_start_size_.x);
            break;
        case ResizeHandle(5):
            size_.x += delta.x;
            if (proportional) size_.y = size_.x * (resize_start_size_.y / resize_start_size_.x);
            break;
        case ResizeHandle(6):
            size_.x -= delta.x;
            size_.y += proportional ? delta.x : delta.y;
            break;
        case ResizeHandle(7):
            size_.y += delta.y;
            if (proportional) size_.x = size_.y * (resize_start_size_.x / resize_start_size_.y);
            break;
        case ResizeHandle(8):
            size_.x += delta.x;
            size_.y += proportional ? delta.x : delta.y;
            break;
        }

        // Минимальный размер
        size_.x = ImMax(size_.x, 10.0f);
        size_.y = ImMax(size_.y, 10.0f);

        return (size_.x != old_size.x || size_.y != old_size.y);

    }

    nlohmann::json Widget::ToJson() const {
        return {
            {"id", id_},
            {"name", name_},
            {"type", static_cast<int>(type_)},
            {"position", {position_.x, position_.y}},
            {"size", {size_.x, size_.y}},
            {"bg_color", bg_color_},
            {"min_width", min_width_},
            {"min_height", min_height_}
        };
    }

    void Widget::FromJson(const nlohmann::json& json) {
        if (json.contains("name")) name_ = json["name"];
        if (json.contains("position") && json["position"].is_array()) {
            position_.x = json["position"][0];
            position_.y = json["position"][1];
        }
        if (json.contains("size") && json["size"].is_array()) {
            size_.x = json["size"][0];
            size_.y = json["size"][1];
        }
        if (json.contains("bg_color")) bg_color_ = json["bg_color"];
        if (json.contains("min_width")) min_width_ = json["min_width"];
        if (json.contains("min_height")) min_height_ = json["min_height"];
    }


}