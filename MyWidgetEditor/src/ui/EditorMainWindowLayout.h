#pragma once
#include "../managers/WidgetManager.h"
#include <imgui.h>
#include <GLFW/glfw3.h>

class Editor {
private:
    wg::WidgetManager widget_manager_;
   
    // Состояние редактора
    ImVec2 canvas_p0_;
    ImVec2 canvas_size_;
    bool show_grid_ = true;
    float grid_size_ = 20.0f;
    int selected_template_ = -1;

public:
    void Render(bool* p_open, ImGuiViewport* viewport, GLFWwindow* window, std::vector<std::string>& templates_names);

private:
    void RenderMenuBar();
    void RenderLeftPanel(std::vector<std::string>& templates_names);
    void RenderRightPanel();
    void RenderCanvas();
    void RenderWidgetProperties();

    // Обработка событий канваса
    void HandleCanvasInteraction();

    // Вспомогательные методы
    ImVec2 GetMousePosRelativeToCanvas() const;
    void DrawGrid(ImDrawList* draw_list) const;

    // Создание виджетов
    void CreateWidgetFromTemplate(const std::string& type, const ImVec2& position);
};