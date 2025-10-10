#include "WidgetInteraction.h"
#include <imgui_internal.h>


namespace WidgetInteraction
{
    static bool s_is_dragging = false;
    static bool s_is_resizing = false;
    static ResizeHandle s_active_handle = NONE;
    static ImVec2 s_drag_start_pos;
    static ImVec2 s_resize_start_size;

    bool HandleWidgetInteraction(Widget& widget, const ImVec2& canvas_p0, bool is_hovered, bool is_active)
    {
        ImGuiIO& io = ImGui::GetIO();
        ImVec2 mouse_pos = io.MousePos;

        // Конвертируем координаты виджета в экранные координаты
        ImVec2 screen_min = ImVec2(canvas_p0.x + widget.p_min.x, canvas_p0.y + widget.p_min.y);
        ImVec2 screen_max = ImVec2(canvas_p0.x + widget.p_max.x, canvas_p0.y + widget.p_max.y);

        bool widget_modified = false;
        ResizeHandle hovered_handle = NONE;

        // Определяем зоны ресайза (8 точек вокруг виджета)
        const float handle_size = 8.0f;
        ImVec2 handles[8] = {
            {screen_min.x, screen_min.y},                           // TOP_LEFT
            {(screen_min.x + screen_max.x) * 0.5f, screen_min.y},   // TOP
            {screen_max.x, screen_min.y},                           // TOP_RIGHT
            {screen_min.x, (screen_min.y + screen_max.y) * 0.5f},   // LEFT
            {screen_max.x, (screen_min.y + screen_max.y) * 0.5f},   // RIGHT
            {screen_min.x, screen_max.y},                           // BOTTOM_LEFT
            {(screen_min.x + screen_max.x) * 0.5f, screen_max.y},   // BOTTOM
            {screen_max.x, screen_max.y}                            // BOTTOM_RIGHT
        };

        // Проверяем ховер над ручками ресайза
        for (int i = 0; i < 8; i++) {
            ImVec2 handle_pos = handles[i];
            ImRect handle_rect(handle_pos.x - handle_size, handle_pos.y - handle_size,
                handle_pos.x + handle_size, handle_pos.y + handle_size);

            if (handle_rect.Contains(mouse_pos)) {
                hovered_handle = static_cast<ResizeHandle>(i + 1);
                break;
            }
        }

        //// Устанавливаем курсоры
        //if (hovered_handle != NONE) {
        //    const char* cursor = NULL;
        //    switch (hovered_handle) {
        //    case TOP_LEFT: case BOTTOM_RIGHT: cursor = ImGui::GetMouseCursorName(ImGuiMouseCursor_ResizeNWSE); break;
        //    case TOP_RIGHT: case BOTTOM_LEFT: cursor = ImGui::GetMouseCursorName(ImGuiMouseCursor_ResizeNESW); break;
        //    case TOP: case BOTTOM: cursor = ImGui::GetMouseCursorName(ImGuiMouseCursor_ResizeNS); break;
        //    case LEFT: case RIGHT: cursor = ImGui::GetMouseCursorName(ImGuiMouseCursor_ResizeEW); break;
        //    }
        //    // ImGui::SetMouseCursor(ImGuiMouseCursor_ResizeAll); // Можно установить конкретный курсор
        //}

        // Начало перетаскивания/ресайза
        if (ImGui::IsMouseClicked(0) && is_hovered) {
            ImRect widget_rect(screen_min, screen_max);

            if (hovered_handle != NONE) {
                // Начало ресайза
                s_is_resizing = true;
                s_active_handle = hovered_handle;
                s_resize_start_size = ImVec2(widget.p_max.x - widget.p_min.x, widget.p_max.y - widget.p_min.y);
                s_drag_start_pos = mouse_pos;
            }
            else if (widget_rect.Contains(mouse_pos)) {
                // Начало перетаскивания
                s_is_dragging = true;
                s_drag_start_pos = mouse_pos;
            }
        }

        // Обработка перетаскивания
        if (s_is_dragging && ImGui::IsMouseDragging(0)) {
            ImVec2 delta = ImGui::GetMouseDragDelta(0);
            ImVec2 new_min = ImVec2(s_drag_start_pos.x - canvas_p0.x + delta.x - (s_drag_start_pos.x - screen_min.x),
                s_drag_start_pos.y - canvas_p0.y + delta.y - (s_drag_start_pos.y - screen_min.y));

            float width = widget.p_max.x - widget.p_min.x;
            float height = widget.p_max.y - widget.p_min.y;

            widget.p_min = new_min;
            widget.p_max = ImVec2(new_min.x + width, new_min.y + height);
            widget_modified = true;
        }

        // Обработка ресайза с пропорциональным масштабированием
        if (s_is_resizing && ImGui::IsMouseDragging(0)) {
            ImVec2 delta = ImGui::GetMouseDragDelta(0);
            ImVec2 new_size = s_resize_start_size;

            // Пропорциональное масштабирование при зажатом Shift
            bool proportional = io.KeyShift;

            switch (s_active_handle) {
            case TOP_LEFT:
                new_size.x -= delta.x;
                new_size.y -= proportional ? delta.x : delta.y;
                break;
            case TOP:
                new_size.y -= delta.y;
                if (proportional) new_size.x = new_size.y * (s_resize_start_size.x / s_resize_start_size.y);
                break;
            case TOP_RIGHT:
                new_size.x += delta.x;
                new_size.y += proportional ? delta.x : delta.y;
                break;
            case LEFT:
                new_size.x -= delta.x;
                if (proportional) new_size.y = new_size.x * (s_resize_start_size.y / s_resize_start_size.x);
                break;
            case RIGHT:
                new_size.x += delta.x;
                if (proportional) new_size.y = new_size.x * (s_resize_start_size.y / s_resize_start_size.x);
                break;
            case BOTTOM_LEFT:
                new_size.x -= delta.x;
                new_size.y += proportional ? delta.x : delta.y;
                break;
            case BOTTOM:
                new_size.y += delta.y;
                if (proportional) new_size.x = new_size.y * (s_resize_start_size.x / s_resize_start_size.y);
                break;
            case BOTTOM_RIGHT:
                new_size.x += delta.x;
                new_size.y += proportional ? delta.x : delta.y;
                break;
            }

            // Минимальный размер
            new_size.x = ImMax(new_size.x, 10.0f);
            new_size.y = ImMax(new_size.y, 10.0f);

            // Применяем изменения к виджету
            switch (s_active_handle) {
            case TOP_LEFT:
                widget.p_min.x = widget.p_max.x - new_size.x;
                widget.p_min.y = widget.p_max.y - new_size.y;
                break;
            case TOP:
                widget.p_min.y = widget.p_max.y - new_size.y;
                break;
            case TOP_RIGHT:
                widget.p_max.x = widget.p_min.x + new_size.x;
                widget.p_min.y = widget.p_max.y - new_size.y;
                break;
            case LEFT:
                widget.p_min.x = widget.p_max.x - new_size.x;
                break;
            case RIGHT:
                widget.p_max.x = widget.p_min.x + new_size.x;
                break;
            case BOTTOM_LEFT:
                widget.p_min.x = widget.p_max.x - new_size.x;
                widget.p_max.y = widget.p_min.y + new_size.y;
                break;
            case BOTTOM:
                widget.p_max.y = widget.p_min.y + new_size.y;
                break;
            case BOTTOM_RIGHT:
                widget.p_max.x = widget.p_min.x + new_size.x;
                widget.p_max.y = widget.p_min.y + new_size.y;
                break;
            }

            widget_modified = true;
        }

        // Завершение операций
        if (ImGui::IsMouseReleased(0)) {
            s_is_dragging = false;
            s_is_resizing = false;
            s_active_handle = NONE;
        }

        return widget_modified;
    }

    void DrawWidget(const Widget& widget, const ImVec2& canvas_p0, ImDrawList* draw_list)
    {
        ImVec2 widget_pmin_screen = ImVec2(canvas_p0.x + widget.p_min.x, canvas_p0.y + widget.p_min.y);
        ImVec2 widget_pmax_screen = ImVec2(canvas_p0.x + widget.p_max.x, canvas_p0.y + widget.p_max.y);

        // Рисуем основной прямоугольник
        draw_list->AddRectFilled(widget_pmin_screen, widget_pmax_screen, IM_COL32(50, 10, 100, 255));
        draw_list->AddRect(widget_pmin_screen, widget_pmax_screen, IM_COL32(255, 255, 255, 255), 0.0f, 0, 2.0f);

        // Текст
        ImVec2 text_pos = ImVec2(widget_pmin_screen.x + 5, widget_pmin_screen.y + 5);
        draw_list->AddText(text_pos, IM_COL32(255, 255, 255, 255), widget.Name.c_str());

        // Рисуем ручки ресайза (только если виджет активен или ховер)
        const float handle_size = 4.0f;
        ImVec2 handles[8] = {
            widget_pmin_screen,                                                                     // TOP_LEFT
            ImVec2((widget_pmin_screen.x + widget_pmax_screen.x) * 0.5f, widget_pmin_screen.y),     // TOP
            ImVec2(widget_pmax_screen.x, widget_pmin_screen.y),                                     // TOP_RIGHT
            ImVec2(widget_pmin_screen.x, (widget_pmin_screen.y + widget_pmax_screen.y) * 0.5f),     // LEFT
            ImVec2(widget_pmax_screen.x, (widget_pmin_screen.y + widget_pmax_screen.y) * 0.5f),     // RIGHT
            ImVec2(widget_pmin_screen.x, widget_pmax_screen.y),                                     // BOTTOM_LEFT
            ImVec2((widget_pmin_screen.x + widget_pmax_screen.x) * 0.5f, widget_pmax_screen.y),     // BOTTOM
            widget_pmax_screen                                                                      // BOTTOM_RIGHT
        };

        for (int i = 0; i < 8; i++) {
            draw_list->AddRectFilled(
                ImVec2(handles[i].x - handle_size, handles[i].y - handle_size),
                ImVec2(handles[i].x + handle_size, handles[i].y + handle_size),
                IM_COL32(255, 255, 255, 255)
            );
        }
    }
}