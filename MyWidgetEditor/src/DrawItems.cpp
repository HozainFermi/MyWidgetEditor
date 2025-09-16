#include "DrawItems.h"



void DrawItems(std::vector<Widget>& used_assets, ImDrawList* draw_list, ImVec2& canvas_pmin, ImVec2& canvas_pmax)
{
    for (size_t i = 0; i < used_assets.size(); i++)
    {
    // Немедленно отрисовываем прямоугольник на канвасе
    ImVec2 screen_min = ImVec2(canvas_pmin.x + used_assets[i].p_min.x, canvas_pmin.y + used_assets[i].p_min.y);
    ImVec2 screen_max = ImVec2(canvas_pmin.x + used_assets[i].p_max.x, canvas_pmin.y + used_assets[i].p_max.y);
    draw_list->AddRect(screen_min, screen_max, IM_COL32(50, 10, 100, 255), 0.0f, 0, 2.0f);

    // Добавляем текст с именем виджета
    ImVec2 text_pos = ImVec2(screen_min.x + 5, screen_min.y + 5);
    draw_list->AddText(text_pos, IM_COL32(255, 255, 255, 255), used_assets[i].Name.c_str());

    }


}