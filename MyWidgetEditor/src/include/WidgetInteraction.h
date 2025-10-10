#pragma once
#include "Widget.h"
#include <imgui.h>

namespace WidgetInteraction
{
    enum ResizeHandle {
        NONE = 0,
        TOP_LEFT, TOP, TOP_RIGHT,
        LEFT, RIGHT,
        BOTTOM_LEFT, BOTTOM, BOTTOM_RIGHT
    };

    bool HandleWidgetInteraction(Widget& widget, const ImVec2& canvas_p0, bool is_hovered, bool is_active,int id);
    void DrawWidget(const Widget& widget, const ImVec2& canvas_p0, ImDrawList* draw_list);
}