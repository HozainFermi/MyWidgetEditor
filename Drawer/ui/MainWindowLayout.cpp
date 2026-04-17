#include "MainWindowLayout.h"



namespace rn {
    RuntimeWindowProperties MainWindowLayout::window_props_;


    void MainWindowLayout::Render(bool* p_open, ImGuiViewport* viewport, GLFWwindow* window,RuntimeWidgetManager* manager)
    {
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);

        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, window_props_.rounding);
        if (ImGui::Begin("main_window", p_open,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize   |
            ImGuiWindowFlags_NoBackground)) {

        ImDrawList* drawlist = ImGui::GetWindowDrawList();
        ImVec2 main_screen_max = { ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y };
        ImVec2 main_screen_min = ImGui::GetWindowPos();
        ImVec2 window_size = { main_screen_max.x - main_screen_min.x, main_screen_max.y - main_screen_min.y };

            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, window_props_.rounding);
            if(ImGui::BeginChild("main_child", ImVec2(ImGui::GetContentRegionAvail().x, ImGui::GetContentRegionAvail().y)) )
            {   
                background_scene_.SCR_WIDTH = ImGui::GetWindowSize().x;
                background_scene_.SCR_HEIGHT = ImGui::GetWindowSize().y;

                if (window_props_.frag_GLSLshader_file != "" && window_props_.vertex_GLSLshader_file != "") {
                    // Сохраняем текущий Viewport, чтобы не сбить ImGui
                    GLint last_viewport[4];
                    glGetIntegerv(GL_VIEWPORT, last_viewport);

                    // Рендерим сцену в FBO
                    glBindFramebuffer(GL_FRAMEBUFFER, background_scene_.FBO);
                    glViewport(0, 0, (GLsizei)ImGui::GetWindowSize().x, (GLsizei)ImGui::GetWindowSize().y);

                    glClearColor(manager->window_props_.bg_color_float[0],
                        manager->window_props_.bg_color_float[1],
                        manager->window_props_.bg_color_float[2],
                        manager->window_props_.bg_color_float[3]);
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                    if (!background_scene_.models_.empty()) {

                        auto model = glm::mat4(1.0f);
                        auto view = glm::mat4(1.0f);
                        auto proj = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

                        background_scene_.models_[0].model_mat = model;


                        background_scene_.models_[0].shader->setFloat("iTime", (float)glfwGetTime());
                        background_scene_.models_[0].shader->setVec2("iResolution", window_size.x, window_size.y);
                        background_scene_.models_[0].shader->setMat4("ortho_model", model);
                        background_scene_.models_[0].shader->setMat4("ortho_view", view);
                        background_scene_.models_[0].shader->setMat4("ortho_projection", proj);

                    }

                    background_scene_.Draw();
                    glBindFramebuffer(GL_FRAMEBUFFER, 0);

                    // Восстанавливаем Viewport обратно для ImGui
                    glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);

                    ImVec2 pos = ImVec2(main_screen_min.x + 0, main_screen_min.y + 0);
                    ImGui::PushID("background_scene");
                    ImGui::GetWindowDrawList()->AddImageRounded(
                        (ImTextureID)(intptr_t)background_scene_.textureColorBuffer,
                        pos,
                        ImVec2(main_screen_max.x, main_screen_max.y),
                        ImVec2(0, 1), ImVec2(1, 0),
                        IM_COL32_WHITE,
                        manager->window_props_.rounding
                    );
                    ImGui::PopID();
                }
                //ImVec2 screen_max = { ImGui::GetWindowPos().x + ImGui::GetWindowSize().x, ImGui::GetWindowPos().y + ImGui::GetWindowSize().y };
                drawlist->AddRectFilled(main_screen_min, main_screen_max, manager->window_props_.bg_color,manager->window_props_.rounding);
                manager->UpdateAll();
                manager->RenderAll(drawlist);
                manager->RenderContentAll();                      
            }
            ImGui::EndChild();
            ImGui::PopStyleVar();

        }       
        ImGui::End();
        ImGui::PopStyleVar();
    }
}