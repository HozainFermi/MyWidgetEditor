#include "EditorMainWindowLayout.h"
#include "DrawItems.h"
#include <imgui.h>
#include <stdio.h>
#include <imgui_internal.h>
#include <vector>
#include "Widget.h"

void ShowMainWindowLayout(bool* p_open, bool* use_grid, ImGuiViewport* viewport, std::vector<Widget>& assets, std::vector<Widget>& used_assets ,ImGuiIO& io)
{
    //ImDrawList* draw_list = ImGui::GetWindowDrawList();
    static ImVec2 last_valid_mouse_rel = ImVec2(0, 0);
    static ImVec2 canvas_size_actual = ImVec2(0, 0);
   // static ImVec2 canvas_minpoint_screen = ImVec2(0, 0);
   
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    if (ImGui::Begin("Widget Editor", p_open,
        ImGuiWindowFlags_MenuBar |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoCollapse))
    {
        // Menu bar
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem("Close", "Ctrl+W")) { *p_open = false; }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        // Главный контейнер
        ImGui::BeginChild("MainContainer", ImVec2(0, 0));
        {
            static int selected_widget = -1; // Выбранный виджет

            ImGui::BeginChild("LeftPanel", ImVec2(ImGui::GetContentRegionAvail().x * 0.7f, 0),
                ImGuiChildFlags_ResizeX);
            {
                if (selected_widget >= 0 && selected_widget < assets.size())
                {
                    if (ImGui::BeginTabBar("MainTabs"))
                    {
                        if (ImGui::BeginTabItem("Workspace"))
                        {
                            ImGui::Text("Editing: %s", assets[selected_widget].Name.c_str());

                            // Редактирование текста выбранного виджета
                            if (ImGui::InputTextMultiline("##source",
                                assets[selected_widget].EditBuffer,
                                IM_ARRAYSIZE(assets[selected_widget].EditBuffer),
                                ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16),
                                ImGuiInputTextFlags_AllowTabInput))
                            {
                                // Обновляем Info при изменении текста
                                assets[selected_widget].Info = assets[selected_widget].EditBuffer;
                            }
                            ImGui::EndTabItem();
                        }

                        if (ImGui::BeginTabItem("Properties"))
                        {
                            ImGui::Text("Properties for: %s", assets[selected_widget].Name.c_str());
                            ImGui::Text("Type: %s", assets[selected_widget].Type.c_str());
                            ImGui::Text("Start Position: (%.1f, %.1f) ",
                                assets[selected_widget].p_min.x,
                                assets[selected_widget].p_min.y
                                                         
                            );
                            ImGui::Text("Size x: %.1f", assets[selected_widget].p_max.x - assets[selected_widget].p_min.x);
                            ImGui::Text("Size y: %.1f", assets[selected_widget].p_max.y - assets[selected_widget].p_min.y);

                            ImGui::EndTabItem();
                        }
                        ImGui::EndTabBar();
                    }
                }
                else
                {
                    ImGui::Text("Select a widget from the right panel");
                }
            }
            ImGui::EndChild();

            ImGui::SameLine();

            // Правая панель
            ImGui::BeginChild("RightPanel", ImVec2(150, 0), ImGuiChildFlags_Borders);
            {

                //begin tab bar assets
                //tab item  cur using in widget 
                ImGui::Text("Assets");
                ImGui::Separator();

                for (int i = 0; i < assets.size(); i++)
                {
                    // Отображаем имя виджета
                    if (ImGui::Selectable(assets[i].Name.c_str(), selected_widget == i))
                    {
                        selected_widget = i; // Запоминаем выбранный виджет
                    }

                    // Drag & Drop источник
                    if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_None))
                    {
                        // Передаём индекс виджета
                        
                        ImGui::SetDragDropPayload("WIDGET_DND", &i, sizeof(int));
                        ImGui::Text("Dragging: %s", assets[i].Name.c_str());
                        ImGui::EndDragDropSource();
                    }

                    // Drag & Drop цель (для перестановки)
                    if (ImGui::BeginDragDropTarget())
                    {
                        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("WIDGET_DND"))
                        {
                            IM_ASSERT(payload->DataSize == sizeof(int));
                            int payload_index = *(const int*)payload->Data;
                            // логика перестановки ассетов
                            Widget tmp = assets[i];
                            assets[i] = assets[payload_index];
                            assets[payload_index] = tmp;

                            

                        }
                        ImGui::EndDragDropTarget();
                    }
                }
            }
            ImGui::EndChild();
            ImGui::SameLine();
            

            //Рабочая область
            ImGui::BeginChild("WidgetPanel", ImVec2(0, 0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX | ImGuiChildFlags_ResizeY);
            {
                
                ImGui::Text("Working Area");
                ImGui::Checkbox("Grid", use_grid);
                ImGui::Text("Mouse position: (x:%.1f, y:%.1f)", last_valid_mouse_rel.x, last_valid_mouse_rel.y);
                //ImGui::SameLine();
                ImGui::Text("Canvas size: (x:%.1f, y:%.1f) ", canvas_size_actual.x, canvas_size_actual.y);
                

                // Создаем невидимую кнопку-канвас для перехвата взаимодействий
                ImVec2 canvas_size = ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y);
               //canvas_minpoint_screen = ImGui::GetCursorScreenPos();

                ImGui::InvisibleButton("canvas", canvas_size, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
                const bool is_hovered = ImGui::IsItemHovered();
                const bool is_active = ImGui::IsItemActive();

                // Получаем позицию и размеры канваса в экранных координатах
                ImVec2 canvas_p0_screen = ImGui::GetItemRectMin();
                ImVec2 canvas_p1_screen = ImGui::GetItemRectMax();
                canvas_size_actual = ImVec2(canvas_p1_screen.x - canvas_p0_screen.x, canvas_p1_screen.y - canvas_p0_screen.y);

                // Получаем текущую позицию мыши в экранных координатах
                ImVec2 mouse_pos = io.MousePos;

                // Вычисляем координаты мыши ОТНОСИТЕЛЬНО ЛЕВОГО ВЕРХНЕГО УГЛА КАНВАСА
                ImVec2 mouse_pos_rel(0, 0);
                if (ImGui::IsMousePosValid() && is_hovered) // Проверяем, что мышь над канвасом
                {
                    mouse_pos_rel.x = mouse_pos.x - canvas_p0_screen.x;
                    mouse_pos_rel.y = mouse_pos.y - canvas_p0_screen.y;
                    last_valid_mouse_rel = mouse_pos_rel; // Обновляем последние валидные координаты
                }
                else
                {
                    // Если мышь не над канвасом, используем последние валидные координаты
                    mouse_pos_rel = last_valid_mouse_rel;
                }

        
                ImDrawList* draw_list = ImGui::GetWindowDrawList();
                // Рисуем фон канваса
                draw_list->AddRectFilled(canvas_p0_screen, canvas_p1_screen, IM_COL32(50, 50, 50, 255));
                draw_list->AddRect(canvas_p0_screen, canvas_p1_screen, IM_COL32(255, 255, 255, 255));
                DrawItems(use_grid,used_assets,draw_list,canvas_p0_screen, canvas_p1_screen);

                // Обрабатываем Drag & Drop на канвасе
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("WIDGET_DND"))
                    {
                        IM_ASSERT(payload->DataSize == sizeof(int));
                        int payload_index = *(const int*)payload->Data;

                        // Вычисляем позицию относительно канваса
                        ImVec2 drop_pos_rel = ImVec2(mouse_pos.x - canvas_p0_screen.x, mouse_pos.y - canvas_p0_screen.y);

                        // Создаем новый виджет в позиции дропа 
                        Widget new_used_asset = assets[payload_index];
                       new_used_asset.p_min = drop_pos_rel;
                       new_used_asset.p_max = ImVec2(drop_pos_rel.x + 50, drop_pos_rel.y + 30);                                               
                        used_assets.push_back(new_used_asset);
                    }
                    ImGui::EndDragDropTarget();
                }

            }
            ImGui::EndChild();



        }
        ImGui::EndChild();
    }
    ImGui::End();
}