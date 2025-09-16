#include "EditorMainWindowLayout.h"
#include <imgui.h>
#include <stdio.h>
#include <imgui_internal.h>
#include <vector>
#include "Widget.h"

void ShowMainWindowLayout(bool* p_open, ImGuiViewport* viewport, std::vector<Widget>& assets, std::vector<Widget> used_assets ,ImGuiIO& io)
{
    //ImDrawList* draw_list = ImGui::GetWindowDrawList();

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
        ImGui::BeginChild("MainContainer", ImVec2(0, 0), ImGuiChildFlags_Borders);
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
            ;

            //Рабочая область
            ImGui::BeginChild("WidgetPanel", ImVec2(0, 0), ImGuiChildFlags_AlwaysAutoResize | ImGuiChildFlags_Borders);
            {
                
                ImGui::Text("Working Area");

                ImVec2 panel_pos = ImGui::GetWindowPos();
                ImVec2 panel_size = ImGui::GetWindowSize();
                ImVec2 mouse_pos = io.MousePos;

                ImVec2 mouse_pos_rel = ImVec2(mouse_pos.x - panel_pos.x, mouse_pos.y - panel_pos.y);

                if (ImGui::IsMousePosValid())
                    ImGui::Text("Mouse Position: (%.1f,%.1f)", mouse_pos_rel.x, mouse_pos_rel.y);
                else
                    ImGui::Text("Mouse Position: <invalid>");

                // Создаем невидимую кнопку-канвас для перехвата взаимодействий
                ImVec2 canvas_size = ImVec2(ImGui::GetContentRegionAvail().x, 400);
                ImGui::InvisibleButton("canvas", canvas_size, ImGuiButtonFlags_MouseButtonLeft | ImGuiButtonFlags_MouseButtonRight);
                const bool is_hovered = ImGui::IsItemHovered();
                const bool is_active = ImGui::IsItemActive();

                // Получаем позицию канваса
                ImVec2 canvas_p0 = ImGui::GetItemRectMin();
                ImVec2 canvas_p1 = ImGui::GetItemRectMax();

                // Получаем draw_list для этой панели
                ImDrawList* draw_list = ImGui::GetWindowDrawList();

                // Рисуем фон канваса
                draw_list->AddRectFilled(canvas_p0, canvas_p1, IM_COL32(50, 50, 50, 255));
                draw_list->AddRect(canvas_p0, canvas_p1, IM_COL32(255, 255, 255, 255));


                // Обрабатываем Drag & Drop на канвасе
                if (ImGui::BeginDragDropTarget())
                {
                    if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("WIDGET_DND"))
                    {
                        IM_ASSERT(payload->DataSize == sizeof(int));
                        int payload_index = *(const int*)payload->Data;

                        // Вычисляем позицию относительно канваса
                        ImVec2 drop_pos_rel = ImVec2(mouse_pos.x - canvas_p0.x, mouse_pos.y - canvas_p0.y);

                        // Создаем новый виджет в позиции дропа
                        //Widget new_widget = assets[payload_index];
                        assets[payload_index].p_min = drop_pos_rel;
                        assets[payload_index].p_max = ImVec2(drop_pos_rel.x + 50, drop_pos_rel.y + 30);
                        //assets.push_back(assets[payload_index]);
                        // Немедленно отрисовываем прямоугольник на канвасе
                        ImVec2 screen_min = ImVec2(canvas_p0.x + assets[payload_index].p_min.x, canvas_p0.y + assets[payload_index].p_min.y);
                        ImVec2 screen_max = ImVec2(canvas_p0.x + assets[payload_index].p_max.x, canvas_p0.y + assets[payload_index].p_max.y);
                        draw_list->AddRect(screen_min, screen_max, IM_COL32(50, 10, 100, 255), 0.0f, 0, 2.0f);

                        // Добавляем текст с именем виджета
                        ImVec2 text_pos = ImVec2(screen_min.x + 5, screen_min.y + 5);
                        draw_list->AddText(text_pos, IM_COL32(255, 255, 255, 255), assets[payload_index].Name.c_str());
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