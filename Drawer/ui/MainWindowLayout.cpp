#include "MainWindowLayout.h"



namespace rn {
    core::WindowProperties MainWindowLayout::window_props_;


    void MainWindowLayout::Render(bool* p_open, ImGuiViewport* viewport, GLFWwindow* window,RuntimeWidgetManager* manager)
    {
        ImGui::SetNextWindowPos(viewport->WorkPos);
        ImGui::SetNextWindowSize(viewport->WorkSize);

        // Без отступов и рамки: содержимое должно занимать всё окно, иначе по краям виден фон
        ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, window_props_.rounding);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
        if (ImGui::Begin("main_window", p_open,
            ImGuiWindowFlags_NoTitleBar |
            ImGuiWindowFlags_NoCollapse |
            ImGuiWindowFlags_NoResize   |
            ImGuiWindowFlags_NoBackground)) {

            ImGui::PushStyleVar(ImGuiStyleVar_ChildRounding, window_props_.rounding);
            if (ImGui::BeginChild("main_child", ImVec2(0.0f, 0.0f), ImGuiChildFlags_None, ImGuiWindowFlags_NoBackground))
            {
                // Всё рисуем в draw list этого child: фон -> шейдер -> виджеты.
                // Раньше виджеты рисовались в родительское окно, и child с шейдером перекрывал текст.
                ImDrawList* drawlist = ImGui::GetWindowDrawList();
                ImVec2 main_screen_min = ImGui::GetWindowPos();
                ImVec2 window_size = ImGui::GetWindowSize();
                ImVec2 main_screen_max = { main_screen_min.x + window_size.x, main_screen_min.y + window_size.y };

                drawlist->AddRectFilled(main_screen_min, main_screen_max, manager->window_props_.bg_color, manager->window_props_.rounding);

                background_scene_.SCR_WIDTH = window_size.x;
                background_scene_.SCR_HEIGHT = window_size.y;

                if (window_props_.frag_GLSLshader_file != "" && window_props_.vertex_GLSLshader_file != "" && !background_scene_.models_.empty()) {

                    background_scene_.ResizeFramebuffer(background_scene_.SCR_WIDTH, background_scene_.SCR_HEIGHT);
                    // Сохраняем текущий Viewport, чтобы не сбить ImGui
                    GLint last_viewport[4];
                    glGetIntegerv(GL_VIEWPORT, last_viewport);

                    // Рендерим сцену в FBO
                    glBindFramebuffer(GL_FRAMEBUFFER, background_scene_.FBO);
                    glViewport(0, 0, (GLsizei)window_size.x, (GLsizei)window_size.y);

                    glClearColor(manager->window_props_.bg_color_float[0],
                        manager->window_props_.bg_color_float[1],
                        manager->window_props_.bg_color_float[2],
                        manager->window_props_.bg_color_float[3]);
                    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

                    auto model = glm::mat4(1.0f);
                    auto view = glm::mat4(1.0f);
                    auto proj = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

                    background_scene_.models_[0].model_mat = model;
                    background_scene_.models_[0].shader->setFloat("iTime", (float)glfwGetTime());
                    background_scene_.models_[0].shader->setVec2("iResolution", window_size.x, window_size.y);
                    background_scene_.models_[0].shader->setMat4("ortho_model", model);
                    background_scene_.models_[0].shader->setMat4("ortho_view", view);
                    background_scene_.models_[0].shader->setMat4("ortho_projection", proj);

                    background_scene_.Draw();
                    glBindFramebuffer(GL_FRAMEBUFFER, 0);

                    // Восстанавливаем Viewport обратно для ImGui
                    glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);

                    drawlist->AddImageRounded(
                        (ImTextureID)(intptr_t)background_scene_.textureColorBuffer,
                        main_screen_min,
                        main_screen_max,
                        ImVec2(0, 1), ImVec2(1, 0),
                        IM_COL32_WHITE,
                        manager->window_props_.rounding
                    );
                }

                // Курсор берём у GLFW, а не у ImGui: в режиме обоев окно не получает событий мыши
                double cursor_x = 0.0, cursor_y = 0.0;
                glfwGetCursorPos(window, &cursor_x, &cursor_y);
                manager->UpdateAll(main_screen_min, ImVec2((float)cursor_x, (float)cursor_y), ImGui::GetIO().DeltaTime);
                manager->RenderAll(drawlist, main_screen_min);
                manager->RenderContentAll(main_screen_min);
            }
            ImGui::EndChild();
            ImGui::PopStyleVar();
        }
        ImGui::End();
        ImGui::PopStyleVar(3);
    }
}