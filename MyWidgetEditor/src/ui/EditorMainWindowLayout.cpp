// ui/EditorMainWindowLayout.cpp
#include "EditorMainWindowLayout.h"
#include "../widgets/TextWidget.h"
#include "../widgets/InputTextWidget.h"
//#include "../widgets/ButtonWidget.h"
// ... другие виджеты

// Реализация методов класса Editor
void Editor::RenderMenuBar() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Save", "Ctrl+S")) {
                widget_manager_.SaveToFile("layout.json");
            }
            if (ImGui::MenuItem("Load", "Ctrl+L")) {
                widget_manager_.LoadFromFile("layout.json");
            }
            if (ImGui::MenuItem("Exit")) {
                // Логика выхода
            }
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("Edit")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) {}
            if (ImGui::MenuItem("Redo", "Ctrl+Y")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Select All", "Ctrl+A")) {}
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View")) {
            ImGui::MenuItem("Show Grid", nullptr, &show_grid_);
            ImGui::EndMenu();
        }

        ImGui::EndMenuBar();
    }
}

void Editor::RenderLeftPanel() {
    ImGui::Text("Widget Templates");
    ImGui::Separator();

    // Список доступных виджетов
    const char* templates[] = {
        "Text", "InputText", "Button", "Slider",
        "Checkbox", "ComboBox", "ColorPicker"
    };

    for (int i = 0; i < IM_ARRAYSIZE(templates); i++) {
        if (ImGui::Selectable(templates[i], selected_template_ == i)) {
            selected_template_ = i;
        }

        // Drag & Drop источник
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoDisableHover)) {
            ImGui::SetDragDropPayload("WIDGET_TEMPLATE", templates[i], strlen(templates[i]) + 1);
            ImGui::Text("Add %s", templates[i]);
            ImGui::EndDragDropSource();
        }
    }

    ImGui::Separator();
    ImGui::Text("Drag widgets to canvas");
}

void Editor::RenderRightPanel() {
    wg::Widget* selected = widget_manager_.GetSelectedWidget();

    if (selected) {
        ImGui::Text("Properties: %s", selected->GetName().c_str());
        ImGui::Separator();

        // Позиция
        ImVec2 pos = selected->GetPosition();
        if (ImGui::DragFloat2("Position", &pos.x, 1.0f)) {
            selected->SetPosition(pos);
        }

        // Размер
        ImVec2 size = selected->GetSize();
        if (ImGui::DragFloat2("Size", &size.x, 1.0f, 10.0f, 1000.0f)) {
            selected->SetSize(size);
        }

        // Имя
        static char name_buffer[256];
        strcpy_s(name_buffer, selected->GetName().c_str());
        if (ImGui::InputText("Name", name_buffer, sizeof(name_buffer))) {
            selected->SetName(name_buffer);
        }

        // Тип виджета (только для чтения)
        ImGui::Text("Type: %d", (int)selected->GetType());

        ImGui::Separator();

        // Кнопки действий
        if (ImGui::Button("Delete", ImVec2(-1, 0))) {
            widget_manager_.DeleteWidget(selected->GetId());
        }

        if (ImGui::Button("Duplicate", ImVec2(-1, 0))) {
            // Логика дублирования
        }

        if (ImGui::Button("Bring to Front", ImVec2(-1, 0))) {
            // Логика поднятия наверх
        }
    }
    else {
        ImGui::Text("No widget selected");
        ImGui::Separator();
        ImGui::Text("Click on a widget to select it");
        ImGui::Text("Right-click for context menu");
    }
}

ImVec2 Editor::GetMousePosRelativeToCanvas() const {
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 mouse_pos = io.MousePos;
   
    ImVec2 relative_pos(
        mouse_pos.x - canvas_p0_.x,
        mouse_pos.y - canvas_p0_.y
    );

    // Проверяем, находится ли relative_pos внутри размеров канваса
    if (relative_pos.x < 0 || relative_pos.x > canvas_size_.x ||
        relative_pos.y < 0 || relative_pos.y > canvas_size_.y) {
        return ImVec2(0, 0);
    }

    return relative_pos;
}

void Editor::CreateWidgetFromTemplate(const std::string& type, const ImVec2& position) {
    static int widget_counter = 1;

    if (type == "Text") {
        std::string name = "Text_" + std::to_string(widget_counter++);
        widget_manager_.CreateWidget<wg::TextWidget>(name, position, "Sample Text");
    }
    else if (type == "InputText") {
        std::string name = "Input_" + std::to_string(widget_counter++);
        widget_manager_.CreateWidget<wg::InputTextWidget>(name, position);
    }
    // Добавь другие типы по мере необходимости

    // Сразу выделяем созданный виджет
    if (widget_manager_.GetCount() > 0) {
        // widget_manager_.SelectWidget() // Нужен метод для выделения
    }
}

void Editor::DrawGrid(ImDrawList* draw_list) const {
    if (!show_grid_ || grid_size_ <= 0) return;

    ImU32 grid_color = IM_COL32(100, 100, 100, 50);
    ImU32 major_grid_color = IM_COL32(150, 150, 150, 80);

    // Вертикальные линии
    for (float x = canvas_p0_.x; x < canvas_p0_.x + canvas_size_.x; x += grid_size_) {
        bool is_major = fmod(x - canvas_p0_.x, grid_size_ * 5.0f) < 1.0f;
        draw_list->AddLine(
            ImVec2(x, canvas_p0_.y),
            ImVec2(x, canvas_p0_.y + canvas_size_.y),
            is_major ? major_grid_color : grid_color,
            1.0f
        );
    }

    // Горизонтальные линии
    for (float y = canvas_p0_.y; y < canvas_p0_.y + canvas_size_.y; y += grid_size_) {
        bool is_major = fmod(y - canvas_p0_.y, grid_size_ * 5.0f) < 1.0f;
        draw_list->AddLine(
            ImVec2(canvas_p0_.x, y),
            ImVec2(canvas_p0_.x + canvas_size_.x, y),
            is_major ? major_grid_color : grid_color,
            1.0f
        );
    }
}

// Реализация основного метода Render (если его нет)
void Editor::Render(bool* p_open, ImGuiViewport* viewport, GLFWwindow* window) {
    ImGui::SetNextWindowPos(viewport->WorkPos);
    ImGui::SetNextWindowSize(viewport->WorkSize);

    if (ImGui::Begin("Widget Editor", p_open,
        ImGuiWindowFlags_MenuBar |
        ImGuiWindowFlags_NoTitleBar |
        ImGuiWindowFlags_NoCollapse |
        ImGuiWindowFlags_NoResize)) {

        RenderMenuBar();

        // Главный сплиттер
        ImGui::BeginChild("MainSplitter", ImVec2(0, 0));
        {
            // Левая панель (20%)
            ImGui::BeginChild("LeftPanel", ImVec2(ImGui::GetContentRegionAvail().x * 0.2f, 0),
                ImGuiChildFlags_ResizeX | ImGuiChildFlags_Borders);
            RenderLeftPanel();
            ImGui::EndChild();

            ImGui::SameLine();

            // Центральная панель (60%)
            ImGui::BeginChild("CanvasPanel", ImVec2(ImGui::GetContentRegionAvail().x * 0.6f, 0),
                ImGuiChildFlags_ResizeX |ImGuiChildFlags_ResizeY | ImGuiChildFlags_Borders);
            RenderCanvas();
            ImGui::EndChild();

            ImGui::SameLine();

            // Правая панель (20%)
            ImGui::BeginChild("RightPanel", ImVec2(0, 0), ImGuiChildFlags_Borders);
            RenderRightPanel();
            ImGui::EndChild();
        }
        ImGui::EndChild();
    }
    ImGui::End();
}

// Реализация RenderCanvas (если её нет)
void Editor::RenderCanvas() {
    ImGui::Text("Canvas (Ctrl+S to save, Ctrl+L to load)");   
    ImGui::Text("Grid size");
    
    ImGui::Checkbox("Grid", &show_grid_);
    ImGui::SameLine();    
    ImGui::DragFloat("", &grid_size_, 1.0f, 5.0f, 100.0f);
    ImGui::Spacing();
    ImVec2 curmouse= GetMousePosRelativeToCanvas();
    
    // Информация о канвасе
    ImGui::Text("Canvas: %.0fx%.0f", canvas_size_.x, canvas_size_.y);
    ImGui::SameLine();
    ImGui::Text("Mouse: %.0f, %.0f",
       curmouse.x,
       curmouse.y);
    
    // Создаём невидимую кнопку-канвас
    ImVec2 canvas_avail = ImGui::GetContentRegionAvail();
    ImGui::InvisibleButton("CanvasButton", canvas_avail,
        ImGuiButtonFlags_MouseButtonLeft |
        ImGuiButtonFlags_MouseButtonRight);

    // Получаем координаты канваса
    canvas_p0_ = ImGui::GetItemRectMin();
    ImVec2 canvas_p1 = ImGui::GetItemRectMax();
    canvas_size_ = ImVec2(canvas_p1.x - canvas_p0_.x, canvas_p1.y - canvas_p0_.y);
    
    // DrawList для отрисовки
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    // Фон канваса
    draw_list->AddRectFilled(canvas_p0_, canvas_p1, IM_COL32(40, 40, 40, 255));

    // Сетка
    DrawGrid(draw_list);

    // Рамка канваса
    draw_list->AddRect(canvas_p0_, canvas_p1, IM_COL32(100, 100, 100, 255), 0.0f, 0, 2.0f);

    // Обработка Drag & Drop
    if (ImGui::BeginDragDropTarget()) {
        if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("WIDGET_TEMPLATE")) {
            const char* widget_type = (const char*)payload->Data;
            ImVec2 drop_pos = GetMousePosRelativeToCanvas();
            CreateWidgetFromTemplate(widget_type, drop_pos);
        }
        ImGui::EndDragDropTarget();
    }
    // === Обновляем и рисуем все виджеты ===
    widget_manager_.UpdateAll(canvas_p0_);
    widget_manager_.RenderAll(draw_list, canvas_p0_);
    widget_manager_.RenderContentAll();


    
}