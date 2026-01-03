#pragma once
#include "../managers/WidgetManager.h"
#include <imgui.h>
#include <GLFW/glfw3.h>

enum class FileBrowserMode {
    Load,
    Run,
    Save,
    None
};


class Editor {
private:
    wg::WidgetManager widget_manager_;
   
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
    FileBrowserMode browsermode;
    static Editor* instance_;



public:

    Editor() {                
    }
    static Editor& Get() {
        static Editor instance;
        return instance;
    }

    void Render(bool* p_open, ImGuiViewport* viewport, GLFWwindow* window, std::vector<std::string>& templates_names);
    
   
    void ScanConfigFiles() {
        all_files_.clear();
        for (const auto& entry : std::filesystem::directory_iterator("./configs")) {
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
        auto& editor = Editor::Get();

        if (data->EventFlag == ImGuiInputTextFlags_CallbackEdit ||
            data->EventFlag == ImGuiInputTextFlags_CallbackAlways)
        {
            editor.SetSearchQuery(data->Buf);
        }

        return 0;
    }
   
    void OnFileForLoadSelected(std::string filename);
    void OnFileForRunSelected(std::string filename);
    

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

    // Создание виджетов
    void CreateWidgetFromTemplate(const std::string& type, const ImVec2& position);
};