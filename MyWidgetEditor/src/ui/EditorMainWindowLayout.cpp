#include "EditorMainWindowLayout.h"
#include "../widgets/TextWidget.h"
#include "../widgets/InputTextWidget.h"
#include <iostream>
#include "../widgets/TableWidget.h"
#include "../src/managers/WidgetFactory.h"
#include "RuntimeWindowProperties.h"
#include <filesystem>
//#include "../widgets/ButtonWidget.h"
// ... другие виджеты

static void HelpMarker(const char* desc)
{
    ImGui::TextDisabled("(?)");
    if (ImGui::BeginItemTooltip())
    {
        ImGui::PushTextWrapPos(ImGui::GetFontSize() * 35.0f);
        ImGui::TextUnformatted(desc);
        ImGui::PopTextWrapPos();
        ImGui::EndTooltip();
    }
}

void Editor::OnFileForLoadSelected(const std::string& filename) {
    widget_manager_.LoadFromFile("./configs/"+filename+".json");
}
void Editor::OnFileForRunSelected(const std::string& filename) {

    std::string relativeStr = "./configs/" + filename + ".json";
    std::filesystem::path relativePath(relativeStr);
    std::filesystem::path absPath = std::filesystem::absolute(relativePath);

    std::string command = "Drawer.exe \"" + absPath.string() + "\"";

    std::cout << "Executing: " << command << "\n";

    int result = std::system(command.c_str());

    if (result != 0) {
        std::cerr << "Error launching second app: " << result << "\n";
    }
}



void Editor::RenderFileBrowser(FileBrowserMode& mode) {
    if (filebrowser_open_) {        
        ImGui::OpenPopup("FileBrowser");
        if (ImGui::BeginPopupModal("FileBrowser", NULL,
            ImGuiWindowFlags_AlwaysAutoResize))
        {
            // Строка поиска
            static char search_buffer[256] = "";

            // Синхронизируем buffer с search_query_
            if (ImGui::IsWindowAppearing()) {
                strncpy_s(search_buffer, search_query_.c_str(), sizeof(search_buffer) - 1);
            }

            if (ImGui::InputText("##Search", search_buffer, sizeof(search_buffer),
                ImGuiInputTextFlags_CallbackAlways,
                &Editor::SearchCallback))
            {
                // InputText возвращает true при редактировании
                SetSearchQuery(search_buffer);
            }

            // Подсказка
            ImGui::SameLine();
            ImGui::TextDisabled("(?)");
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("Type to filter files\nPress Enter to open selected");
            }

            ImGui::Separator();

            // Список файлов
            const auto& files = GetFilteredFiles();
            static int selected = -1;

            if (ImGui::BeginChild("FileList", ImVec2(400, 300), true)) {
                if (files.empty()) {
                    ImGui::TextColored(ImVec4(1, 0.5f, 0.5f, 1),
                        search_query_.empty() ?
                        "No config files found in ./configs/" :
                        "No files matching '%s'", search_query_.c_str());
                }
                else {
                    for (int i = 0; i < files.size(); i++) {
                        if (ImGui::Selectable(files[i].c_str(), selected == i)) {
                            selected = i;
                        }

                        // Двойной клик для быстрого открытия
                        if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(0)) {                            
                            ImGui::CloseCurrentPopup();
                            if (mode == FileBrowserMode::Run) {                                
                                OnFileForRunSelected(files[selected]);

                            }
                            else if (mode == FileBrowserMode::Load) {
                                OnFileForLoadSelected(files[selected]);
                            }
                            mode = FileBrowserMode::None;
                            filebrowser_open_ = false;
                        }
                    }
                }
                ImGui::EndChild();

            }

            // Статистика
            ImGui::Text("%zu files (%zu total)", files.size(), all_files_.size());

            ImGui::Separator();

            // Кнопки действий
            if (ImGui::Button("Refresh")) {
                ScanConfigFiles();
                selected = -1;
            }

            ImGui::SameLine();

            if (ImGui::Button("Open") && selected >= 0 && selected < files.size()) {
                ImGui::CloseCurrentPopup();
                filebrowser_open_ = false;
                if (mode == FileBrowserMode::Run) {                    
                    OnFileForRunSelected(files[selected]);
                }
                else if (mode == FileBrowserMode::Load) {
                    OnFileForLoadSelected(files[selected]);
                }
                mode = FileBrowserMode::None;
            }

            ImGui::SameLine();

            if (ImGui::Button("Cancel")) {
                filebrowser_open_ = false;
                mode = FileBrowserMode::None;
                search_query_.clear();
                filter_dirty_ = true;
                strcpy_s(search_buffer, "");
                ImGui::CloseCurrentPopup();
            }

            // Горячие клавиши
            if (ImGui::IsWindowFocused() && !ImGui::IsAnyItemActive()) {
                if (ImGui::IsKeyPressed(ImGuiKey_Escape)) {
                    ImGui::CloseCurrentPopup();
                    mode = FileBrowserMode::None;
                    filebrowser_open_ = false;
                }
                if (ImGui::IsKeyPressed(ImGuiKey_Enter) && selected >= 0 && selected < files.size()) {
                    if (mode == FileBrowserMode::Run) {
                        //OnFileSelected(files[selected]);
                    }
                    else if (mode == FileBrowserMode::Load) {
                        OnFileForLoadSelected(files[selected]);
                    }
                    ImGui::CloseCurrentPopup();
                    mode = FileBrowserMode::None;
                }
            }

            ImGui::EndPopup();
        }
    }
}

void Editor::RenderSaveFileMenu() {
    if (filesave_open_) {
        static char savefile_buffer[32];
        ImGui::OpenPopup("SaveFile");
        if (ImGui::BeginPopupModal("SaveFile", NULL,
            ImGuiWindowFlags_AlwaysAutoResize)) {

            if (ImGui::BeginChild("SaveFileChild", ImVec2(400, 300), true)) {

                ImGui::InputText("", savefile_buffer,IM_ARRAYSIZE(savefile_buffer));
                ImGui::SameLine();
                ImGui::Text(".json");               
                std::string temp;
                if (ImGui::Button("Save")) {
                    temp = savefile_buffer;
                    auto it = std::find(all_files_.begin(), all_files_.end(), temp);
                    if (it!=all_files_.end()) {
                        ImGui::TextColored(ImVec4(1, 0.5f, 0.5f, 1), "Config file with name '%s' already exists", it );
                    }
                    else {
                        widget_manager_.SaveToFile("./configs/"+temp+".json");
                    }
                }
                ImGui::SameLine();
                if (ImGui::Button("Cancel")) {
                    ImGui::CloseCurrentPopup();
                    filebrowser_open_ = false;
                    filesave_open_ = false;                    
                }
                ImGui::EndChild();
            }
            ImGui::EndPopup();
        }
    }
}

// Реализация методов класса Editor
void Editor::RenderMenuBar() {
    if (ImGui::BeginMenuBar()) {
        if (ImGui::BeginMenu("File")) {
            if (ImGui::MenuItem("Run", "")) {          
                filebrowser_open_ = true;
                browsermode = FileBrowserMode::Run;
            }
            if (ImGui::MenuItem("Save", "Ctrl+S")) {
                filesave_open_ = true;                
            }
            if (ImGui::MenuItem("Load", "Ctrl+L")) {                
                filebrowser_open_ = true;
                browsermode = FileBrowserMode::Load;
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
    RenderFileBrowser(browsermode);
    RenderSaveFileMenu();
        
}



void Editor::RenderLeftPanel(std::vector<std::string>& templates) {
    ImGui::Text("Widget Templates");
    ImGui::SameLine();
    HelpMarker("Drag widgets to canvas");
    ImGui::Separator();

       
    for (int i = 0; i < templates.size(); i++) {
        if (ImGui::Selectable(templates[i].c_str(), selected_template_ == i)) {
            selected_template_ = i;
        }
       
        // Drag & Drop источник
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoDisableHover)) {
            ImGui::SetDragDropPayload("WIDGET_TEMPLATE", templates[i].c_str(), strlen(templates[i].c_str()) + 1);
            ImGui::Text("Add %s", templates[i]);
            ImGui::EndDragDropSource();
        }
    }

    ImGui::Separator();
    auto widgetsptrs = widget_manager_.GetAllWidgets();
    for (int i = 0; i < widget_manager_.GetCount(); i++) {
       if(ImGui::Selectable(widgetsptrs[i]->GetId().c_str(), widgetsptrs[i]->IsSelected())) {
           widgetsptrs[i]->SetSelected(true);
       }
    }
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
        ImGui::Text("Type: %s", selected->TypeToString(selected->GetType()).c_str());

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
            widget_manager_.BringToFront(selected->GetId());
        }
        ImGui::Separator();        
        selected->RenderProperties();        
        
    }
    else {       
        static char FRAGbuf[150];
        static char VERTbuf[150];
        ImGui::Text("Main window Properties:");
        ImGui::Separator();
        ImGui::Text("Width: %.0f", canvas_size_.x);
        ImGui::SameLine();
        ImGui::Text("Height: %.0f", canvas_size_.y);
        ImGui::Text("Background color:");
        ImGui::ColorEdit4("MainWindowColor", (float*)&window_props_.bg_color, ImGuiColorEditFlags_DisplayHSV);
        ImGui::Checkbox("Always on top", &window_props_.always_on_top);

        if (ImGui::InputText("Frag Shader", FRAGbuf, IM_ARRAYSIZE(FRAGbuf))) {
            window_props_.frag_GLSLshader_file = FRAGbuf;
        }
        if (ImGui::InputText("Vert Shader", VERTbuf, IM_ARRAYSIZE(VERTbuf))) {
            window_props_.vertex_GLSLshader_file = VERTbuf;
        }

        ImGui::Separator();
        ImGui::Text("No widget selected");        
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

void Editor::CreateWidgetFromTemplate(const std::string& widget_class, const ImVec2& position) {
    static int widget_counter = 1; 
    std::string name = widget_class + std::to_string(widget_counter++);
   
    // Создаем через фабрику
    std::unique_ptr<wg::Widget> widget = wg::WidgetFactory::CreateWithName(widget_class, name, position);

    if (widget) {
        // Добавляем в менеджер
        widget_manager_.AddWidget(std::move(widget));        
    }
    else {
        // Обработка ошибки: виджет не зарегистрирован
        std::cerr << "Widget class not registered: " << widget_class << std::endl;
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
void Editor::Render(bool* p_open, ImGuiViewport* viewport, GLFWwindow* window, std::vector<std::string>& templates) {
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
                        
            RenderLeftPanel(templates);
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
            wg::Widget* selected = widget_manager_.GetSelectedWidget();
            RenderRightPanel();
            
            if (selected) { selected->SetStaySelected(ImGui::IsWindowHovered()); }
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
            const char* widget_class = (const char*)payload->Data;
            ImVec2 drop_pos = GetMousePosRelativeToCanvas();
            CreateWidgetFromTemplate(widget_class, drop_pos);
        }
        ImGui::EndDragDropTarget();
    }
    // === Обновляем и рисуем все виджеты ===
    widget_manager_.UpdateAll(canvas_p0_, canvas_size_);
    widget_manager_.RenderAll(draw_list, canvas_p0_);
    widget_manager_.RenderContentAll(canvas_p0_);


    
}