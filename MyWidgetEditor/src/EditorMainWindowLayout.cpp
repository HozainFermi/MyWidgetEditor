#include "EditorMainWindowLayout.h"
#include "DrawItems.h"
#include <imgui.h>
#include <stdio.h>
#include <imgui_internal.h>
#include <vector>
#include "Widget.h"

static void ShowMenuFile();

void ShowMainWindowLayout(bool* p_open, bool* use_grid, ImGuiViewport* viewport, std::vector<Widget>& assets, std::vector<Widget>& used_assets ,ImGuiIO& io)
{
    
    static ImVec2 last_valid_mouse_rel = ImVec2(0, 0);
    static ImVec2 canvas_size_actual = ImVec2(0, 0);
    static int selected_widget = -1;        // Выбранный виджет
    static int selected_used_widget = -1;   // Выбранный используемый виджет
  
   
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    if (ImGui::Begin("Widget Editor", p_open,
        ImGuiWindowFlags_MenuBar |
        ImGuiWindowFlags_NoBringToFrontOnFocus |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize))
    {
        // Menu bar
        if (ImGui::BeginMenuBar())
        {
            if (ImGui::BeginMenu("File"))
            {
                ShowMenuFile();
                //if (ImGui::MenuItem("Close", "Ctrl+W")) { *p_open = false; }
                ImGui::EndMenu();
            }
            ImGui::EndMenuBar();
        }

        // Главный контейнер
        ImGui::BeginChild("MainContainer", ImVec2(0, 0));
        {          
            ImGui::BeginChild("LeftPanel", ImVec2(ImGui::GetContentRegionAvail().x * 0.7f, 0),
                ImGuiChildFlags_ResizeX);
            {
                if (selected_used_widget >= 0 && selected_used_widget < used_assets.size())
                {
                    if (ImGui::BeginTabBar("MainTabs"))
                    {
                        if (ImGui::BeginTabItem("Workspace"))
                        {
                            ImGui::Text("Editing: %s", used_assets[selected_used_widget].Name.c_str());

                            // Редактирование текста выбранного виджета
                            if (ImGui::InputTextMultiline("##source",
                                used_assets[selected_used_widget].EditBuffer,
                                IM_ARRAYSIZE(used_assets[selected_used_widget].EditBuffer),
                                ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16),
                                ImGuiInputTextFlags_AllowTabInput))
                            {
                                // Обновляем Info при изменении текста
                                used_assets[selected_used_widget].Info = used_assets[selected_used_widget].EditBuffer;
                            }
                            ImGui::EndTabItem();
                        }

                        if (ImGui::BeginTabItem("Properties"))
                        {
                            ImGui::Text("Properties for: %s", used_assets[selected_used_widget].Name.c_str());
                            ImGui::Text("Type: %s", used_assets[selected_used_widget].Type.c_str());
                            ImGui::Text("Start Position: (%.1f, %.1f) ",
                                used_assets[selected_used_widget].p_min.x,
                                used_assets[selected_used_widget].p_min.y
                                                         
                            );
                            ImGui::Text("Size x: %.1f", used_assets[selected_used_widget].p_max.x - used_assets[selected_used_widget].p_min.x);
                            ImGui::Text("Size y: %.1f", used_assets[selected_used_widget].p_max.y - used_assets[selected_used_widget].p_min.y);

                            ImGui::EndTabItem();
                        }
                        ImGui::EndTabBar();
                    }
                }else if (selected_widget >= 0 && selected_widget < assets.size())
                {
                    if (ImGui::BeginTabBar("MainTabs"))
                    {
                        if (ImGui::BeginTabItem("Description"))
                        {
                            ImGui::Text("Widget: %s", assets[selected_widget].Name.c_str());                          
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
                if (ImGui::CollapsingHeader("Assets")) {
                
                ImGui::Separator();

                for (int i = 0; i < assets.size(); i++)
                {
                    // Отображаем имя виджета
                    if (ImGui::Selectable(assets[i].Name.c_str(), selected_widget == i))
                    {
                        selected_widget = i; // Запоминаем выбранный виджет
                        selected_used_widget = -1;
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
                if (ImGui::CollapsingHeader("Used")) {
                    ImGui::Separator();

                    // Отображаем имя виджета
                    for (int i = 0; i < used_assets.size(); i++) {
                        //std::string used_name = i+"-"+used_assets[i].Name;
                        if (ImGui::Selectable(used_assets[i].Name.c_str(), selected_used_widget == i))
                        {
                            selected_used_widget = i; // Запоминаем выбранный виджет
                            selected_widget = -1;
                        }
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
                        new_used_asset.Name = std::to_string(used_assets.size() + 1) + "-" + new_used_asset.Name;
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

static void ShowMenuFile()
{    
    //ImGui::MenuItem("File", NULL, false, false);
    if (ImGui::MenuItem("New")) {}
    if (ImGui::MenuItem("Open", "Ctrl+O")) {}
    if (ImGui::BeginMenu("Open Recent"))
    {
        ImGui::MenuItem("fish_hat.c");
        ImGui::MenuItem("fish_hat.inl");
        ImGui::MenuItem("fish_hat.h");
        if (ImGui::BeginMenu("More.."))
        {
            ImGui::MenuItem("Hello");
            ImGui::MenuItem("Sailor"); 
            ImGui::EndMenu();
        }
        ImGui::EndMenu();
    }    
    if (ImGui::MenuItem("Save", "Ctrl+S")) {}
    if (ImGui::MenuItem("Save As..")) {}

    ImGui::Separator();
    if (ImGui::BeginMenu("Options"))
    {
        static bool enabled = true;
        ImGui::MenuItem("Enabled", "", &enabled);
        ImGui::BeginChild("child", ImVec2(0, 60), ImGuiChildFlags_Borders);
        for (int i = 0; i < 10; i++)
            ImGui::Text("Scrolling Text %d", i);
        ImGui::EndChild();
        static float f = 0.5f;
        static int n = 0;
        ImGui::SliderFloat("Value", &f, 0.0f, 1.0f);
        ImGui::InputFloat("Input", &f, 0.1f);
        ImGui::Combo("Combo", &n, "Yes\0No\0Maybe\0\0");
        ImGui::EndMenu();
    }
   
    if (ImGui::BeginMenu("Colors"))
    {
        float sz = ImGui::GetTextLineHeight();
        for (int i = 0; i < ImGuiCol_COUNT; i++)
        {
            const char* name = ImGui::GetStyleColorName((ImGuiCol)i);
            ImVec2 p = ImGui::GetCursorScreenPos();
            ImGui::GetWindowDrawList()->AddRectFilled(p, ImVec2(p.x + sz, p.y + sz), ImGui::GetColorU32((ImGuiCol)i));
            ImGui::Dummy(ImVec2(sz, sz));
            ImGui::SameLine();
            ImGui::MenuItem(name);
        }
        ImGui::EndMenu();
    }

    // Here we demonstrate appending again to the "Options" menu (which we already created above)
    // Of course in this demo it is a little bit silly that this function calls BeginMenu("Options") twice.
    // In a real code-base using it would make senses to use this feature from very different code locations.
    if (ImGui::BeginMenu("Options")) // <-- Append!
    {
        static bool b = true;
        ImGui::Checkbox("SomeOption", &b);
        ImGui::EndMenu();
    }

    if (ImGui::BeginMenu("Disabled", false)) // Disabled
    {
        ImGui::EndMenu();
    }
    if (ImGui::MenuItem("Checked", NULL, true)) {}
    ImGui::Separator();
    if (ImGui::MenuItem("Quit", "Alt+F4")) {}
}