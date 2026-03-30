#pragma once
#include <imgui.h>
#include <glad/gl.h> 
#include <GLFW/glfw3.h>
#include "WidgetManager.h"
#include "RuntimeWindowProperties.h"
#include "Connections.h"
#include "Mesh.h"
#include "Scene.hpp"

enum class FileBrowserMode {
    Load,
    Run,
    Save,
    None
};


class Editor {
private:
    wg::WidgetManager widget_manager_;
    

    // связи
    std::vector<PortVisual> port_visuals_;
    bool is_dragging_connection_ = false;
    PortRef drag_from_port_;
   
    // Состояние редактора
    ImVec2 canvas_p0_;
    ImVec2 canvas_size_;
    bool show_grid_ = true;
    float grid_size_ = 20.0f;
    int selected_template_ = -1;
    std::vector<std::string> all_files_;
    std::vector<std::string> filtered_files_;
    std::string search_query_;
    bool filter_dirty_ = true;
    bool filebrowser_open_ = false;
    bool filesave_open_ = false;
    FileBrowserMode browsermode= FileBrowserMode::None;
    wg::RuntimeWindowProperties window_props_{};
    
    Styles::Scene background_scene_{};
    //Helpers::Shader background_shaders_{window_props_.vertex_GLSLshader_file, window_props_.frag_GLSLshader_file};
    //Editor() = default;


public:
    
    Editor()
    {
        std::vector<Helpers::Vertex> vertices_{

        Helpers::Vertex{
        glm::vec3{-1.f,1.f,0.0f},
        glm::vec3{1.f,1.f,1.0f},
        glm::vec2{0.f,1.f} },
        Helpers::Vertex{
        glm::vec3{1.f,1.f,0.f},
        glm::vec3{1.f,1.f,1.0f},
        glm::vec2{1.0f,1.0f} },
        Helpers::Vertex{
        glm::vec3{-1.f,-1.f,0.0f},
        glm::vec3{1.f,1.f,1.0f},
        glm::vec2{0.f,0.f} },

         Helpers::Vertex{
        glm::vec3{1.f,1.f,0.f},
        glm::vec3{1.f,1.f,1.0f},
        glm::vec2{1.0f,1.0f} },
         Helpers::Vertex{
        glm::vec3{-1.f,-1.f,0.0f},
        glm::vec3{1.f,1.f,1.0f},
        glm::vec2{0.f,0.f} },
         Helpers::Vertex{
        glm::vec3{1.0f,-1.f,0.0f},
        glm::vec3{1.f,1.f,1.0f},
        glm::vec2{1.0f,0.0f} },

        };
        std::vector<unsigned int> EBOpoints_ = {
            0,1,2,
            1,2,5
        };

        Helpers::Mesh background_mesh_{ vertices_,EBOpoints_ };
       
        std::unique_ptr model = std::make_unique<Helpers::Model>();        
        model->AddMesh(background_mesh_);

        std::unique_ptr shaders = std::make_unique<Helpers::Shader>( window_props_.vertex_GLSLshader_file, window_props_.frag_GLSLshader_file );

        Styles::ModelData model_data { std::move(model), std::move(shaders) };

        background_scene_.models_.push_back(std::move(model_data));
        //background_scene_.camera_->Position = glm::vec3(0.0f, 0.0f, 3.0f);

    }

    static Editor* Get() {
        static Editor instance;
        return &instance;
    }
    Editor(const Editor&) {
        throw std::logic_error("Can not copy Editor - it`s singleton");
    }
    void operator=(const Editor&) {
        throw std::logic_error("Can not copy Editor - it`s singleton");
    }
    Editor(Editor&&) = delete;
    void operator=(const Editor&&) = delete;

    void Render(bool* p_open, ImGuiViewport* viewport, GLFWwindow* window, std::vector<std::string>& templates_names);
    
   
    void ScanConfigFiles() {
        all_files_.clear();
        for (const auto& entry : std::filesystem::directory_iterator(std::string(PROJECT_SOURCE_DIR)+"/configs")) {
            if (entry.is_regular_file() && entry.path().extension() == ".json") {
                all_files_.push_back(entry.path().stem().string());
            }
        }
        std::sort(all_files_.begin(), all_files_.end());
        filter_dirty_ = true;
    }

    void UpdateFilter() {
        if (!filter_dirty_) return;

        filtered_files_.clear();

        if (search_query_.empty()) {
            filtered_files_ = all_files_;
        }
        else {
            // Регистронезависимый поиск
            std::string query_lower = search_query_;
            std::transform(query_lower.begin(), query_lower.end(),
                query_lower.begin(), ::tolower);

            for (const auto& file : all_files_) {
                std::string file_lower = file;
                std::transform(file_lower.begin(), file_lower.end(),
                    file_lower.begin(), ::tolower);

                if (file_lower.find(query_lower) != std::string::npos) {
                    filtered_files_.push_back(file);
                }
            }
        }
        filter_dirty_ = false;
    }

    void SetSearchQuery(const std::string& query) {
        if (search_query_ != query) {
            search_query_ = query;
            filter_dirty_ = true;
        }
    }

    const std::vector<std::string>& GetFilteredFiles() {
        UpdateFilter();
        return filtered_files_;
    }

    // Callback для ImGui InputText
    static int SearchCallback(ImGuiInputTextCallbackData* data) {
        auto editor = Editor::Get();

        if (data->EventFlag == ImGuiInputTextFlags_CallbackEdit ||
            data->EventFlag == ImGuiInputTextFlags_CallbackAlways)
        {
            editor->SetSearchQuery(data->Buf);
        }

        return 0;
    }
   
    void OnFileForLoadSelected(const std::string& filename);
    void OnFileForRunSelected(const std::string& filename);
    

private:
    void RenderFileBrowser(FileBrowserMode& mode);
    void RenderSaveFileMenu();
    void RenderMenuBar();
    void RenderLeftPanel(std::vector<std::string>& templates_names);
    void RenderRightPanel();
    void RenderCanvas();
    void RenderWindowProperties();
    
    // Обработка событий канваса
    void HandleCanvasInteraction();

    // Вспомогательные методы
    ImVec2 GetMousePosRelativeToCanvas() const;
    void DrawGrid(ImDrawList* draw_list) const;
    void RenderConnections(ImDrawList* draw_list);
    void RenderPortsAndHandleConnections(ImDrawList* draw_list);

    // Сохранение/загрузка конфигурации с учётом connections_
    void SaveConfigWithConnections(const std::string& filename);

    // Создание виджетов
    void CreateWidgetFromTemplate(const std::string& type, const ImVec2& position);
};