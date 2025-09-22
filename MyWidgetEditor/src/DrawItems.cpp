#include "DrawItems.h"



void DrawItems(bool* use_grid,std::vector<Widget>& used_assets, ImDrawList* draw_list, ImVec2& canvas_minpoint_actual,ImVec2& canvas_pmin, ImVec2& canvas_pmax)
{
    if (*use_grid) {
        // Используем фактические границы канваса для рисования линий
        ImVec2 canvas_size = ImVec2(canvas_pmax.x - canvas_minpoint_actual.x,
            canvas_pmax.y - canvas_minpoint_actual.y);

        for (size_t i = 1; i < 5; i++) {
            // Вертикальные линии - от верхней до нижней границы канваса
            float x_pos = canvas_minpoint_actual.x + canvas_size.x * i / 5;
            draw_list->AddLine(ImVec2(x_pos, canvas_minpoint_actual.y),
                ImVec2(x_pos, canvas_pmax.y),
                ImColor(128, 255, 128, 255), 1);

            // Горизонтальные линии - от левой до правой границы канваса
            float y_pos = canvas_minpoint_actual.y + canvas_size.y * i / 5;
            draw_list->AddLine(ImVec2(canvas_minpoint_actual.x, y_pos),
                ImVec2(canvas_pmax.x, y_pos),
                ImColor(128, 255, 128, 255), 1);
        }
    }

    for (size_t i = 0; i < used_assets.size(); i++)
    {
        
        
        // Немедленно отрисовываем прямоугольник на канвасе
        ImVec2 screen_min = ImVec2(canvas_pmin.x + used_assets[i].p_min.x, canvas_pmin.y + used_assets[i].p_min.y);
        ImVec2 screen_max = ImVec2(canvas_pmin.x + used_assets[i].p_max.x, canvas_pmin.y + used_assets[i].p_max.y);
    
    
        
        draw_list->AddRectFilled(screen_min, screen_max, IM_COL32(50, 10, 100, 255), 0.0f, 0);
        // Добавляем текст с именем виджета
        ImVec2 text_pos = ImVec2(screen_min.x + 5, screen_min.y + 5);
        draw_list->AddText(text_pos, IM_COL32(255, 255, 255, 255), used_assets[i].Name.c_str());
    
    


    }

    
}