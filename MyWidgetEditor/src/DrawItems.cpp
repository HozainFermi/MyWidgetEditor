#include "DrawItems.h"
#include "WidgetInteraction.h"



void DrawItems(bool* use_grid,std::vector<Widget>& used_assets, ImDrawList* draw_list,ImVec2& canvas_pmin_screen, ImVec2& canvas_pmax_screen)
{
    if (*use_grid) {
        // Используем фактические границы канваса для рисования линий
        ImVec2 canvas_size = ImVec2(canvas_pmax_screen.x - canvas_pmin_screen.x,
            canvas_pmax_screen.y - canvas_pmin_screen.y);

        for (size_t i = 1; i < 5; i++) {
            // Вертикальные линии - от верхней до нижней границы канваса
            float x_pos = canvas_pmin_screen.x + canvas_size.x * i / 5;
            draw_list->AddLine(ImVec2(x_pos, canvas_pmin_screen.y),
                ImVec2(x_pos, canvas_pmax_screen.y),
                ImColor(128, 255, 128, 255), 1);

            // Горизонтальные линии - от левой до правой границы канваса
            float y_pos = canvas_pmin_screen.y + canvas_size.y * i / 5;
            draw_list->AddLine(ImVec2(canvas_pmin_screen.x, y_pos),
                ImVec2(canvas_pmax_screen.x, y_pos),
                ImColor(128, 255, 128, 255), 1);
        }
    }

    // Виджеты
    bool is_hovered = ImGui::IsItemHovered();
    bool is_active = ImGui::IsItemActive();


    for (size_t i = 0; i < used_assets.size(); i++)
    {
                
        // Обработка взаимодействий
        WidgetInteraction::HandleWidgetInteraction(used_assets[i], canvas_pmin_screen, is_hovered, is_active,i);
        // Отрисовка
        WidgetInteraction::DrawWidget(used_assets[i], canvas_pmin_screen, draw_list);

        //// Немедленно отрисовываем прямоугольник на канвасе
        //ImVec2 screen_min = ImVec2(canvas_pmin_screen.x + used_assets[i].p_min.x, canvas_pmin_screen.y + used_assets[i].p_min.y);
        //ImVec2 screen_max = ImVec2(canvas_pmin_screen.x + used_assets[i].p_max.x, canvas_pmin_screen.y + used_assets[i].p_max.y);
        
        //
        //draw_list->AddRectFilled(screen_min, screen_max, IM_COL32(50, 10, 100, 255), 0.0f, 0);
        //// Добавляем текст с именем виджета
        //ImVec2 text_pos = ImVec2(screen_min.x + 5, screen_min.y + 5);
        //draw_list->AddText(text_pos, IM_COL32(255, 255, 255, 255), used_assets[i].Name.c_str());
    
    


    }

    
}