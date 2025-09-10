#include "EditorMainWindowLayout.h"
#include <imgui.h>
#include <stdio.h>
#include <imgui_internal.h>

void ShowMainWindowLayout(bool* p_open, ImGuiViewport* viewport, char* buf)
{
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    if (ImGui::Begin("Widget Editor", p_open, ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoCollapse))
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
            // Левая панель (ресайзится)
            ImGui::BeginChild("LeftPanel", ImVec2(ImGui::GetContentRegionAvail().x * 0.7f, 0),
                 ImGuiChildFlags_ResizeX);
            {
                // Контент левой панели
                static int selected = 0;               

                // Здесь будет ваш основной контент
                if (ImGui::BeginTabBar("MainTabs"))
                {
                    if (ImGui::BeginTabItem("Workspace"))
                    {
                        ImGui::Text("Widget workspace goes here");
                        ImGui::InputTextMultiline("##source", buf, IM_ARRAYSIZE(buf), ImVec2(-FLT_MIN, ImGui::GetTextLineHeight() * 16), ImGuiInputTextFlags_AllowTabInput);
                        ImGui::EndTabItem();
                    }
                    if (ImGui::BeginTabItem("Properties"))
                    {
                        ImGui::Text("Properties panel");
                        ImGui::EndTabItem();
                    }
                    ImGui::EndTabBar();
                }
              
            }
            ImGui::EndChild();
           

            ImGui::SameLine();

            // Правая панель (фиксированная ширина)
            ImGui::BeginChild("RightPanel", ImVec2(150, 0), ImGuiChildFlags_Borders);
            {
              
                // Список ассетов/виджетов
                ImGui::Text("Assets");
                ImGui::Separator();

                static int selected = 0;
                for (int i = 0; i < 20; i++)
                {
                    char label[128];
                    sprintf_s(label, "Widget %d", i);
                    if (ImGui::Selectable(label, selected == i))
                        selected = i;
                }
            }
            ImGui::EndChild();
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

//#include "EditorMainWindowLayout.h"
//#include <imgui.h>
//#include <stdio.h>
//
//
//void ShowMainWindowLayout(bool* p_open, ImGuiViewport* viewport)
//{
//    //: viewport->Pos
//    //: viewport->Size
//
//    ImGui::SetNextWindowPos(viewport->WorkPos);
//    ImGui::SetNextWindowSize(viewport->WorkSize);  
//    //ImGui::SetNextWindowSize(ImVec2(500, 440), ImGuiCond_FirstUseEver);
//    if (ImGui::Begin("Widget Editor", p_open, ImGuiWindowFlags_MenuBar))
//    {
//       
//        if (ImGui::BeginMenuBar())
//        {
//            if (ImGui::BeginMenu("File"))
//            {
//                if (ImGui::MenuItem("Close", "Ctrl+W")) { *p_open = false; }
//                ImGui::EndMenu();
//            }
//            ImGui::EndMenuBar();
//        }
//
//        // Left
//        static int selected = 0;
//        float x_size = viewport->WorkSize.x / 2.5;
//
//        if (ImGui::BeginChild("asset menu", ImVec2(0,0), ImGuiChildFlags_Borders | ImGuiChildFlags_ResizeX)) {
//           
//            {
//                ImGui::BeginGroup();                //x_size/1.5
//                ImGui::BeginChild("item view", ImVec2(0, -ImGui::GetFrameHeightWithSpacing()), ImGuiChildFlags_AutoResizeX); // Leave room for 1 line below us
//                ImGui::Text("MyObject: %d", selected);
//                ImGui::Separator();
//                if (ImGui::BeginTabBar("##Tabs", ImGuiTabBarFlags_None))
//                {
//                    if (ImGui::BeginTabItem("Description"))
//                    {
//                        ImGui::TextWrapped("Lorem ipsum dolor sit amet, consectetur adipiscing elit, sed do eiusmod tempor incididunt ut labore et dolore magna aliqua. ");
//                        ImGui::EndTabItem();
//                    }
//                    if (ImGui::BeginTabItem("Details"))
//                    {
//                        ImGui::Text("ID: 0123456789");
//                        ImGui::EndTabItem();
//                    }
//                    ImGui::EndTabBar();
//                }
//                ImGui::EndChild();
//                if (ImGui::Button("Revert")) {}
//                ImGui::SameLine();
//                if (ImGui::Button("Save")) {}
//                ImGui::EndGroup();
//            }
//            ImGui::SameLine();
//
//            // Right
//            {
//                ImGui::BeginChild("right pane", ImVec2(150, viewport->WorkSize.y - 55), ImGuiChildFlags_Borders );
//                for (int i = 0; i < 100; i++)
//                {
//                    // FIXME: Good candidate to use ImGuiSelectableFlags_SelectOnNav
//                    char label[128];
//                    sprintf_s(label, "MyObject %d", i);
//                    if (ImGui::Selectable(label, selected == i))
//                        selected = i;
//                }
//                ImGui::EndChild();
//            }
//        }ImGui::EndChild();
//    }
//    ImGui::End();
//}