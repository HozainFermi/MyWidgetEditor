#include "RenderWidget.h"
#include <algorithm>
#include <cmath>
#include <glad/gl.h>
#include <GLFW/glfw3.h>
#include <glm/gtc/matrix_transform.hpp>
#include <imgui_stdlib.h>
#include <IconText.h>
#include "core/WindowProperties.h"

namespace core {

    namespace {
        const std::filesystem::path kShadersFolder = std::string(ASSETS_SOURCE_DIR) + "/shaders";
        const std::filesystem::path kModelsFolder = std::string(ASSETS_SOURCE_DIR) + "/models";

        const char* PathTypeToString(PathType type) {
            switch (type) {
            case PathType::Model: return "Model";
            case PathType::VertexShader: return "VertexShader";
            case PathType::FragmentShader: return "FragmentShader";
            }
            return "Unknown";
        }
    }

    RenderWidget::RenderWidget()
        : Widget(WidgetType::IMAGE, ImVec2(400, 300)),
          model_path_(kModelsFolder.generic_string() + "/backpack/backpack.obj"),
          vert_path_(kShadersFolder.generic_string() + "/backpack/1.model_loading.vert"),
          frag_path_(kShadersFolder.generic_string() + "/backpack/1.model_loading.frag") {
    }

    void RenderWidget::RenderContent(const ImVec2& screen_min, const ImVec2& screen_max)
    {
        ImVec2 widget_size = GetSize();
        scene_.SaveCurrentState();

        scene_.SCR_WIDTH = (unsigned int)widget_size.x;
        scene_.SCR_HEIGHT = (unsigned int)widget_size.y;
        scene_.ResizeFramebuffer((unsigned int)widget_size.x, (unsigned int)widget_size.y);

        glBindFramebuffer(GL_FRAMEBUFFER, scene_.FBO);
        glViewport(0, 0, (GLsizei)widget_size.x, (GLsizei)widget_size.y);

        // Под шейдерным фоном - прозрачный фон, иначе цвет фона окна
        const WindowProperties* window = Context().window;
        if (window && window->frag_GLSLshader_file.empty()) {
            glClearColor(window->bg_color_float[0], window->bg_color_float[1], window->bg_color_float[2], window->bg_color_float[3]);
        }
        else if (window) {
            glClearColor(0.0f, 0.0f, 0.0f, 0.0f);
        }
        else {
            glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
        }
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // Вращение по времени, а не по кадрам: скорость не зависит от FPS
        const double now = glfwGetTime();
        if (last_time_ >= 0.0) {
            angle_ = fmodf(angle_ + rotation_speed_ * (float)(now - last_time_), 360.0f);
        }
        last_time_ = now;

        const glm::mat4 transform = glm::scale(
            glm::rotate(glm::mat4(1.0f), glm::radians(angle_), glm::vec3(0.0f, 1.0f, 0.0f)),
            glm::vec3(scale_));
        for (auto& model : scene_.models_) {
            model.model_mat = transform;
            if (model.shader) {
                for (const auto& [name, value] : shader_params_) {
                    model.shader->setFloat("u_" + name, value);  // шейдер без такого uniform просто проигнорирует
                }
            }
        }

        scene_.Draw();
        scene_.RestorePrevState();
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        ImGui::SetCursorScreenPos(ImVec2(screen_min.x, screen_min.y));
        ImGui::PushID(this);
        if (ImGui::BeginChild("##render", widget_size, false, ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse)) {
            ImGui::Image((ImTextureID)(intptr_t)scene_.textureColorBuffer, ImGui::GetContentRegionAvail(),
                ImVec2(0, 1), ImVec2(1, 0));
        }
        ImGui::EndChild();
        ImGui::PopID();
    }

    void RenderWidget::RenderProperties()
    {
        ImGui::Text("==Render Settings==");
        ImGui::Separator();

        ExplorerButton(model_path_, model_paths_, kModelsFolder, PathType::Model);
        ImGui::SameLine();
        ImGui::InputText("Model path##model", &model_path_);

        ExplorerButton(vert_path_, vert_paths_, kShadersFolder, PathType::VertexShader);
        ImGui::SameLine();
        ImGui::InputText("Vert Shader##model", &vert_path_);

        ExplorerButton(frag_path_, frag_paths_, kShadersFolder, PathType::FragmentShader);
        ImGui::SameLine();
        ImGui::InputText("Frag Shader##model", &frag_path_);

        if (ImGui::Button("Load model")) {
            scene_.AddModel(model_path_, vert_path_, frag_path_, "");
            selected_model_ = (int)scene_.models_.size() - 1;
        }

        if (!scene_.models_.empty()) {
            selected_model_ = std::clamp(selected_model_, 0, (int)scene_.models_.size() - 1);
            if (ImGui::BeginCombo("Select Model", scene_.models_[selected_model_].name.c_str())) {
                for (int i = 0; i < (int)scene_.models_.size(); ++i) {
                    const bool is_selected = (selected_model_ == i);
                    std::string label = scene_.models_[i].name + "##" + std::to_string(i);
                    if (ImGui::Selectable(label.c_str(), is_selected)) {
                        selected_model_ = i;
                    }
                    if (is_selected) {
                        ImGui::SetItemDefaultFocus();
                    }
                }
                ImGui::EndCombo();
            }
        }
        else {
            ImGui::Text("No models loaded");
        }

        ImGui::Separator();
        ImGui::Text("Camera");
        ImGui::Separator();

        if (scene_.camera_) {
            auto& camera = *scene_.camera_;
            ImGui::DragFloat("Zoom:", &camera.Zoom, camera.MouseSensitivity);
            if (ImGui::DragFloat("X offset:", &x_offset_, camera.MouseSensitivity)) {
                camera.ProcessMouseMovement(x_offset_, y_offset_);
            }
            if (ImGui::DragFloat("Y offset:", &y_offset_, camera.MouseSensitivity)) {
                camera.ProcessMouseMovement(x_offset_, y_offset_);
            }

            ImGui::Text("Camera Position:");
            ImGui::DragFloat("X:", &camera.Position.x, 0.01f);
            ImGui::DragFloat("Y:", &camera.Position.y, 0.01f);
            ImGui::DragFloat("Z:", &camera.Position.z, 0.01f);
        }

        ImGui::Separator();
        ImGui::Text("Animation");
        ImGui::DragFloat("Rotation speed (deg/s)", &rotation_speed_, 1.0f, -720.0f, 720.0f);
        ImGui::DragFloat("Scale", &scale_, 0.01f, 0.01f, 100.0f);

        ImGui::Separator();
        ImGui::TextDisabled("Input \"params\": rotation_speed, scale, camera_x/y/z, camera_zoom.");
        ImGui::TextDisabled("Any other number goes to shaders as uniform float u_<name>.");
        if (!shader_params_.empty()) {
            ImGui::Text("Shader uniforms from input:");
            for (const auto& [name, value] : shader_params_) {
                ImGui::BulletText("u_%s = %.3f", name.c_str(), value);
            }
        }
    }

    void RenderWidget::OnInput(const std::string& to_port, const std::string&, const std::vector<WidgetValue>& value)
    {
        if (to_port != "params" || value.empty() || !value[0].is_object()) {
            return;
        }
        // Числа могут прийти и строками (например, из таблицы со статическими данными)
        auto to_float = [](const nlohmann::json& v, float& out) {
            if (v.is_number()) { out = v.get<float>(); return true; }
            if (v.is_string()) {
                try { size_t used = 0; out = std::stof(v.get<std::string>(), &used); return used > 0; }
                catch (...) {}
            }
            return false;
        };

        shader_params_.clear();
        for (auto it = value[0].begin(); it != value[0].end(); ++it) {
            float number = 0.0f;
            if (!to_float(it.value(), number)) continue;

            const std::string& key = it.key();
            if (key == "rotation_speed") rotation_speed_ = number;
            else if (key == "scale") scale_ = (std::max)(0.001f, number);
            else if (key == "camera_x") scene_.camera_->Position.x = number;
            else if (key == "camera_y") scene_.camera_->Position.y = number;
            else if (key == "camera_z") scene_.camera_->Position.z = number;
            else if (key == "camera_zoom") scene_.camera_->Zoom = number;
            else shader_params_[key] = number;
        }
    }

    void RenderWidget::ExplorerButton(std::string& current_path, std::vector<std::filesystem::path>& paths, const std::filesystem::path& folder, PathType type)
    {
        ImGui::PushID(PathTypeToString(type));
        if (ImGui::Button(ICON(ICON_FOLDER), ImVec2(35, 20))) {
            paths.clear();
            for (const auto& subfolder : std::filesystem::directory_iterator(folder)) {
                if (!subfolder.is_directory()) continue;
                for (const auto& file : std::filesystem::directory_iterator(subfolder)) {
                    const auto ext = file.path().extension();
                    const bool matches =
                        (type == PathType::Model && (ext == ".obj" || ext == ".gltf")) ||
                        (type == PathType::VertexShader && ext == ".vert") ||
                        (type == PathType::FragmentShader && ext == ".frag");
                    if (matches) {
                        paths.push_back(file.path());
                    }
                }
            }
            ImGui::OpenPopup("select_explorer_popup");
        }

        if (ImGui::BeginPopup("select_explorer_popup")) {
            for (const auto& path : paths) {
                if (ImGui::Selectable(path.filename().string().c_str())) {
                    current_path = path.generic_string();
                }
            }
            ImGui::EndPopup();
        }
        ImGui::PopID();
    }

    nlohmann::json RenderWidget::ToJson() const
    {
        auto json = Widget::ToJson();
        nlohmann::json models_json = nlohmann::json::array();
        for (const auto& model : scene_.models_) {
            models_json.push_back(model.ToJson());
        }
        json["models"] = models_json;

        json["camera"] = {
            {"x", scene_.camera_->Position.x},
            {"y", scene_.camera_->Position.y},
            {"z", scene_.camera_->Position.z},
            {"zoom", scene_.camera_->Zoom},
            {"yaw", scene_.camera_->Yaw},
            {"pitch", scene_.camera_->Pitch}
        };
        json["rotation_speed"] = rotation_speed_;
        json["scale"] = scale_;
        return json;
    }

    void RenderWidget::FromJson(const nlohmann::json& json)
    {
        Widget::FromJson(json);
        rotation_speed_ = json.value("rotation_speed", rotation_speed_);
        scale_ = (std::max)(0.001f, json.value("scale", scale_));

        auto camera_json = json.value("camera", nlohmann::json::object());
        scene_.camera_->Position.x = camera_json.value("x", 0.0f);
        scene_.camera_->Position.y = camera_json.value("y", 0.0f);
        scene_.camera_->Position.z = camera_json.value("z", 0.0f);
        scene_.camera_->Zoom = camera_json.value("zoom", 45.0f);
        scene_.camera_->SetRotation(camera_json.value("yaw", Helpers::YAW), camera_json.value("pitch", Helpers::PITCH));

        for (const auto& model : json.value("models", nlohmann::json::array())) {
            scene_.AddModel(
                model.value("model_path", ""),
                model.value("vert_shader_path", ""),
                model.value("fragment_shader_path", ""),
                "");
        }
    }
}
