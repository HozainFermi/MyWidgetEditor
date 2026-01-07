#include "MainWindowLayout.h"


namespace rn {
	
    void MainWindowLayout::Render(bool* p_open, ImGuiViewport* viewport, GLFWwindow* window)
    {
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);

        if (ImGui::Begin("Widget Editor", p_open,            
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize)) {


            ImGui::BeginChild("LeftPanel", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y));

            ImGui::Text("Hello");
            
            ImGui::EndChild();

        }
        ImGui::End();
    }
}