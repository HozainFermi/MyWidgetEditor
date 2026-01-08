#include "MainWindowLayout.h"



namespace rn {

    void MainWindowLayout::Render(bool* p_open, ImGuiViewport* viewport, GLFWwindow* window,RuntimeWidgetManager* manager)
    {
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);

        if (ImGui::Begin("main_window", p_open,            
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize   |
            ImGuiWindowFlags_NoBackground)) {

        ImDrawList* drawlist = ImGui::GetWindowDrawList();
        ImVec2 main_screen_max = { ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y };
        ImVec2 main_screen_min = ImGui::GetWindowPos();

            if(ImGui::BeginChild("main_child", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y)))
            {   
                //ImVec2 screen_max = { ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y };
                drawlist->AddRectFilled(main_screen_min, main_screen_max, manager->window_props_.bg_color);
                manager->UpdateAll();
                manager->RenderAll(drawlist);
                manager->RenderContentAll();                      
            }
            ImGui::EndChild();

        }
        ImGui::End();
    }
}