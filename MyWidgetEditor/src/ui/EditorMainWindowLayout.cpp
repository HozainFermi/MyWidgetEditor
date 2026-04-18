#include "EditorMainWindowLayout.h"
#include <iostream>
#include "WidgetFactory.h"
#include "RuntimeWindowProperties.h"
#include "independent_launcher.h"
#include <filesystem>
#include <fstream>
#include <IconText.h>


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
    widget_manager_.LoadFromFile(std::string(PROJECT_SOURCE_DIR)+"/configs/"+filename+".json", window_props_);

}
void Editor::OnFileForRunSelected(const std::string& filename) {
    std::filesystem::path relfilePath(std::string(PROJECT_SOURCE_DIR)+"/configs/" + filename + ".json");
    std::filesystem::path relDrawer("../Drawer/Drawer.exe");
    //C:\Users\Hoz\Desktop\MyWidgetEditor\out\build\x64-debug\Drawer
    std::filesystem::path absExePath = std::filesystem::absolute(relDrawer);
    std::filesystem::path absConfigPath = std::filesystem::absolute(relfilePath);

    // Проверка существования файлов
    if (!std::filesystem::exists(absExePath)) {
        std::cerr << "Error: Executable not found: " << absExePath << "\n";
        return;
    }

    if (!std::filesystem::exists(absConfigPath)) {
        std::cerr << "Error: Config file not found: " << absConfigPath << "\n";
        return;
    }

    std::string exePath = "\"" + absExePath.string() + "\"";
    std::string configPath = "\"" + absConfigPath.string() + "\""; 

    std::cout << "Executing: " << exePath << " " << configPath << std::endl;

    //Раздельная передача
#ifdef _WIN32   
    HANDLE proc = IndependentLauncher::launch(exePath, configPath);
    if (proc== nullptr) {///!!!
        std::cerr << "Failed to launch process\n";
    }
    else {
        Process created (proc,filename);        
        processes_.push_back(std::move(created));
    }
#else
    pid_t pid = IndependentLauncher::launch(exePath, configPath);
    if (pid == 0) {
        std::cerr << "Failed to launch process\n";
    }
    else {
        processes_.push_back(Process(pid,filename));
    }
#endif

}


void Editor::SaveConfigWithConnections(const std::string& filename)
{
    // Базовый json с окном и виджетами
    nlohmann::json json = widget_manager_.ToJson(window_props_);

    // Сериализуем connections_
    nlohmann::json conn = nlohmann::json::array();
    for (const auto& c : widget_manager_.connections_) {
        conn.push_back({
            {"from_widget", c.from.widget_id},
            {"from_port",   c.from.port},
            {"to_widget",   c.to.widget_id},
            {"to_port",     c.to.port}
        });
    }
    json["connections"] = conn;

    std::ofstream file(filename);
    if (file.is_open()) {
        file << json.dump(4);
    }
}

// Реализация методов класса Editor
void Editor::RenderMenuBar() {  
    if (ImGui::BeginMenuBar()) {
        //std::string FILEText = std::string(ICON(ICON_FILE_PLUS)) + " File";
        if (ImGui::BeginMenu("File ")) {
            if (ImGui::MenuItem("Run", "")) {          
                filebrowser_open_ = true;
                browsermode = FileBrowserMode::Run;
            }
            if (ImGui::MenuItem("Save ", "Ctrl+S")) {
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

        if (ImGui::BeginMenu("Edit ")) {
            if (ImGui::MenuItem("Undo", "Ctrl+Z")) {}
            if (ImGui::MenuItem("Redo", "Ctrl+Y")) {}
            ImGui::Separator();
            if (ImGui::MenuItem("Select All", "Ctrl+A")) {}
            ImGui::EndMenu();
        }

        if (ImGui::BeginMenu("View ")) {
            ImGui::MenuItem("Show Grid", nullptr, &show_grid_);
            ImGui::EndMenu();
        }
        ImGui::Spacing();
        if (ImGui::BeginMenu("Processes")) {
                if (ImGui::MenuItem("Kill All",NULL)) {
                    processes_.clear();
                }
            for (size_t i = 0; i < processes_.size(); ++i) {
                char proc_num[32];
                snprintf(proc_num, sizeof(proc_num), "Process %zu: %s", i, processes_[i].GetName().c_str());

                if (ImGui::BeginMenu(proc_num)) {
                    if (ImGui::MenuItem("Kill Process")) {                       
                        processes_.erase(processes_.begin() + i);
                    }                    
                    ImGui::EndMenu(); 
                }
            }
            ImGui::EndMenu();
        }


        ImGui::EndMenuBar();
    }

    Editor::RenderFileBrowser(browsermode);
    Editor::RenderSaveFileMenu();
        
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
            ImGui::Text("Add %s", templates[i].c_str());
            ImGui::EndDragDropSource();
        }
    }

    ImGui::Separator();
    ImGui::Text("Used Widgets");
    ImGui::SameLine();
    HelpMarker("Click to select");
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

    static bool* selections[] = 
    { 
      &window_props_.full_screen,
      &window_props_.always_on_top,
      &window_props_.always_on_bottom,
      &window_props_.window_rounding,
      &window_props_.resizeble,
      &window_props_.mouse_passthrougth,
      &window_props_.moveble,
      &window_props_.decorated
    };
    static const char* items[] = {"Full screen" ,"Always on top", "Always on bottom", "Window rounding", "Resizeble", "Mouse passthrougth", "Moveble" ,"Decorated"};
    static char VERTbuf[150];
    static char FRAGbuf[150]; // = window_props_.frag_GLSLshader_file.c_str();
    static std::filesystem::path shadersFolderPath = std::string(ASSETS_SOURCE_DIR) + "/shaders";
    static std::vector<std::string> shaderNames {};
    static std::vector<std::filesystem::path> shaders_paths {};

    std::strncpy(FRAGbuf, window_props_.frag_GLSLshader_file.c_str(), sizeof(FRAGbuf) - 1);    
    FRAGbuf[sizeof(FRAGbuf) - 1] = '\0';

    ImGui::Text("==Main window Properties:==");
    ImGui::Separator();
    ImGui::Text("Width: %.0f", canvas_size_.x);
    ImGui::SameLine();
    ImGui::Text("Height: %.0f", canvas_size_.y);
    ImGui::Text("Background color:");

   
    if (ImGui::ColorEdit4("MainWindowColor", window_props_.bg_color_float, ImGuiColorEditFlags_DisplayHSV)) {

        window_props_.bg_color = ImGui::ColorConvertFloat4ToU32(
            ImVec4(window_props_.bg_color_float[0],
                window_props_.bg_color_float[1],
                window_props_.bg_color_float[2],
                window_props_.bg_color_float[3]));
    }

    if (ImGui::BeginCombo(".", "Window properties"))
    {
        for (int n = 0; n < IM_ARRAYSIZE(items); n++)
        {            
            if (ImGui::Checkbox(items[n], selections[n])){
                if( *selections[1] && *selections[2])  {
                    if (n == 1) {
                        *selections[2] = false;
                    }
                    if (n == 2) {
                        *selections[1] = false;
                    }
                }
            }
        }
        ImGui::EndCombo();
    }
    //window_props_.SetProperties(selections);

    if (selections[3]) {
        ImGui::DragFloat("Rounding",&window_props_.rounding,0.5f,0.0f,500.0f);
    }

    //ImGui::Checkbox("Always on top", &window_props_.always_on_top);
    //ImGui::Checkbox("Moveble", &window_props_.moveble);
    if (ImGui::Button(ICON(ICON_FOLDER), ImVec2(30,15)) ) {
        shaderNames.clear();
        shaders_paths.clear();
        for (const auto& folder : std::filesystem::directory_iterator(shadersFolderPath) )
        {
            if (folder.is_directory()) {
                for (const auto& file : std::filesystem::directory_iterator(folder)) {

                    if (file.path().extension() == ".frag") {                       
                        shaders_paths.push_back(file.path());                       
                        shaderNames.push_back( folder.path().filename().string() );                                                
                    }                    
                }
            }
        }
        ImGui::OpenPopup("select_shader_popup");
    }
    ImGui::SameLine();
    if (ImGui::InputText("Frag Shader", FRAGbuf, IM_ARRAYSIZE(FRAGbuf) )) {
        window_props_.frag_GLSLshader_file = FRAGbuf;        
    }

    if (ImGui::BeginPopup("select_shader_popup")) {
        for (size_t i = 0; i < shaderNames.size(); ++i) {
            if (ImGui::Selectable(shaderNames[i].c_str())) { window_props_.frag_GLSLshader_file = shaders_paths[i].generic_string(); }
        }
        ImGui::EndPopup();                
    }

    if (ImGui::Button("Compile shader")) {
        std::unique_ptr shaders = std::make_unique<Helpers::Shader>(window_props_.vertex_GLSLshader_file, window_props_.frag_GLSLshader_file);
        background_scene_.models_[0].shader.release();
        background_scene_.models_[0].shader = std::move(shaders);
    }
    //if (ImGui::InputText("Vert Shader", VERTbuf, IM_ARRAYSIZE(VERTbuf))) {
    //    window_props_.vertex_GLSLshader_file = VERTbuf;
    //}
    ImGui::Separator();

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
            selected = nullptr;
            widget_manager_.SelectWidget(-1);
            widget_manager_.DeselectAll();
        }

        if (ImGui::Button("Duplicate", ImVec2(-1, 0))) {
            // Логика дублирования
        }

        if (ImGui::Button("Bring to Front", ImVec2(-1, 0))) {
            // Логика поднятия наверх
            widget_manager_.BringToFront(selected->GetId());
        }
        ImGui::Separator();
        if (selected) {
            selected->RenderProperties();        
        }
        
    }
    else {       
        
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

// Реализация основного метода Render
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


// Реализация RenderCanvas 
void Editor::RenderCanvas() {
    
    ImGui::Text("Canvas (Ctrl+S to save, Ctrl+L to load)");   
    ImGui::Text("Grid size");
    
    ImGui::Checkbox("Grid", &show_grid_);
    ImGui::SameLine();     
    ImGui::DragFloat(" ", &grid_size_, 1.0f, 5.0f, 100.0f);
    
    ImGui::Spacing();
    ImVec2 curmouse= GetMousePosRelativeToCanvas();

    // Информация о канвасе
    ImGui::Text("Canvas: %.0fx%.0f", canvas_size_.x, canvas_size_.y);
    window_props_.width = canvas_size_.x;
    window_props_.height = canvas_size_.y;
    ImGui::SameLine();
    ImGui::Text("Mouse: %.0f, %.0f",
       curmouse.x,
       curmouse.y);
    
    // Создаём невидимую кнопку-канвас
    ImVec2 canvas_avail = ImGui::GetContentRegionAvail();
    ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, window_props_.rounding);
    ImGui::InvisibleButton("CanvasButton", canvas_avail,
        ImGuiButtonFlags_MouseButtonLeft |
        ImGuiButtonFlags_MouseButtonRight);
    ImGui::PopStyleVar();

    // Получаем координаты канваса
    canvas_p0_ = ImGui::GetItemRectMin();
    ImVec2 canvas_p1 = ImGui::GetItemRectMax();
    canvas_size_ = ImVec2(canvas_p1.x - canvas_p0_.x, canvas_p1.y - canvas_p0_.y);
    
    // DrawList для отрисовки
    ImDrawList* draw_list = ImGui::GetWindowDrawList();

    //IM_COL32(40, 40, 40, 255)
    // Фон канваса
    ImVec4 to_im32
    { 
        window_props_.bg_color_float[0] * 255,
        window_props_.bg_color_float[1] * 255,
        window_props_.bg_color_float[2] * 255,
        window_props_.bg_color_float[3] * 255 
    };
    ImU32 col = IM_COL32(to_im32.x, to_im32.y, to_im32.z, to_im32.w);    
    draw_list->AddRectFilled(canvas_p0_, canvas_p1, col, window_props_.rounding);
    
    //========================================================================================================
    if (window_props_.frag_GLSLshader_file != "" && window_props_.vertex_GLSLshader_file != "") {        
        background_scene_.SCR_WIDTH = canvas_size_.x;
        background_scene_.SCR_HEIGHT = canvas_size_.y;

        // Сохраняем текущий Viewport, чтобы не сбить ImGui
        GLint last_viewport[4];
        glGetIntegerv(GL_VIEWPORT, last_viewport);

        // Рендерим сцену в FBO
        glBindFramebuffer(GL_FRAMEBUFFER, background_scene_.FBO);
        glViewport(0, 0, (GLsizei)canvas_size_.x, (GLsizei)canvas_size_.y);

        glClearColor(window_props_.bg_color_float[0], window_props_.bg_color_float[1], window_props_.bg_color_float[2], window_props_.bg_color_float[3]);
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        if (!background_scene_.models_.empty()) {

            auto model = glm::mat4(1.0f);
            auto view = glm::mat4(1.0f);
            auto proj = glm::ortho(-1.0f, 1.0f, -1.0f, 1.0f, -1.0f, 1.0f);

            background_scene_.models_[0].model_mat = model;


            background_scene_.models_[0].shader->setFloat("iTime", (float)glfwGetTime());
            background_scene_.models_[0].shader->setVec2("iResolution", canvas_size_.x, canvas_size_.y);
            background_scene_.models_[0].shader->setMat4("ortho_model", model);
            background_scene_.models_[0].shader->setMat4("ortho_view", view);
            background_scene_.models_[0].shader->setMat4("ortho_projection", proj);
            //glm::rotate(background_scene_.models_[0].model_mat,
            //    glm::radians(sin(static_cast<float>(glfwGetTime()))),
            //    glm::vec3(0.0f, 1.0f, 0.0f));
        background_scene_.Draw();        
        glBindFramebuffer(GL_FRAMEBUFFER, 0);

        // Восстанавливаем Viewport обратно для ImGui
        glViewport(last_viewport[0], last_viewport[1], (GLsizei)last_viewport[2], (GLsizei)last_viewport[3]);

        ImVec2 pos = ImVec2(canvas_p0_.x + 0, canvas_p0_.y + 0);

        ImGui::PushID("background_scene");
        ImGui::GetWindowDrawList()->AddImage(
            (ImTextureID)(intptr_t)background_scene_.textureColorBuffer,
            pos,
            ImVec2(pos.x + canvas_size_.x, pos.y + canvas_size_.y),
            ImVec2(0, 1), ImVec2(1, 0)
        );
        ImGui::PopID();
        }

        
        
    }
    //========================================================================================================

    // Сетка
    DrawGrid(draw_list);
  
    // Рамка канваса
    draw_list->AddRect(canvas_p0_, canvas_p1, IM_COL32(100, 100, 100, 255), window_props_.rounding, 0, 2.0f);

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

    // Порты и соединения поверх виджетов
    RenderPortsAndHandleConnections(draw_list);
    RenderConnections(draw_list);
    
    
}

void Editor::RenderPortsAndHandleConnections(ImDrawList* draw_list)
{
    port_visuals_.clear();

    auto widgets = widget_manager_.GetAllWidgets();
    ImGuiIO& io = ImGui::GetIO();
    ImVec2 mouse_pos = io.MousePos;

    const float radius = 6.0f;
    const float spacing = 15.0f;
    const float hit_radius = 6.0f;

    // Сначала собираем все порты и рисуем кружки
    for (auto* w : widgets) {
        if (!w) continue;
        ImRect rect(
            w->GetScreenMin(canvas_p0_),
            w->GetScreenMax(canvas_p0_));

        auto inputs = w->GetInputPorts();
        auto outputs = w->GetOutputPorts();
        
        // Входы слева
        for (int i = 0; i < (int)inputs.size(); ++i) {
            ImVec2 p(
                rect.Min.x-3.0f,
                rect.Min.y + 20.0f + i * spacing);
            PortVisual vis{ {inputs[i].name, w->GetId()}, p, true };
            port_visuals_.push_back(vis);
            draw_list->AddCircleFilled(p, radius, IM_COL32(180, 220, 180, 255));

            // Создаем невидимую кнопку поверх порта
            ImGui::SetCursorScreenPos(ImVec2(vis.pos.x - hit_radius, vis.pos.y - hit_radius));
            ImGui::PushID(&vis);  // Уникальный ID для каждой кнопки
            if (ImGui::InvisibleButton("##port", ImVec2(hit_radius * 2, hit_radius * 2))) {
                // Обработка клика
                //is_dragging_connection_ = true;
                //drag_from_port_ = vis.ref;
            }

            // Тултип при наведении
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s", vis.ref.port.c_str());
            }
            ImGui::PopID();

        }

        // Выходы справа
        for (int i = 0; i < (int)outputs.size(); ++i) {
            ImVec2 p(
                rect.Max.x+3.0f,
                rect.Min.y + 20.0f + i * spacing);
            PortVisual vis{ {outputs[i].name, w->GetId()}, p, false };
            port_visuals_.push_back(vis);
            draw_list->AddCircleFilled(p, radius, IM_COL32(220, 180, 180, 255));
            // Создаем невидимую кнопку поверх порта
            ImGui::SetCursorScreenPos(ImVec2(vis.pos.x - hit_radius, vis.pos.y - hit_radius));
            ImGui::PushID(&vis);  // Уникальный ID для каждой кнопки
            if (ImGui::InvisibleButton("##port", ImVec2(hit_radius * 2, hit_radius * 2))) {
                // Обработка клика
                //is_dragging_connection_ = true;
                //drag_from_port_ = vis.ref;
            }

            // Тултип при наведении
            if (ImGui::IsItemHovered()) {
                ImGui::SetTooltip("%s", vis.ref.port.c_str());
            }
            ImGui::PopID();
        }
    }

    // Обработка drag для создания соединений             
        if (ImGui::IsMouseClicked(0)) {
        // Начало drag'а по порту
        for (auto& vis : port_visuals_) {
            float dx = mouse_pos.x - vis.pos.x;
            float dy = mouse_pos.y - vis.pos.y;
            if (dx * dx + dy * dy <= hit_radius * hit_radius) {
                is_dragging_connection_ = true;
                drag_from_port_ = vis.ref;               
                break;
            }
        }
    }

    

    if (is_dragging_connection_) {
        // Временная линия от исходного порта до мыши
        ImVec2 from_pos{};
        for (auto& vis : port_visuals_) {
            if (vis.ref.widget_id == drag_from_port_.widget_id &&
                vis.ref.port == drag_from_port_.port) {
                from_pos = vis.pos;
                break;
            }
        }
        float dx = (mouse_pos.x - from_pos.x) * 0.5f;
        ImVec2 c1(from_pos.x + dx, from_pos.y);
        ImVec2 c2(mouse_pos.x - dx, mouse_pos.y);
        draw_list->AddBezierCubic(from_pos, c1, c2, mouse_pos,
            IM_COL32(200, 200, 100, 255), 2.0f);

        if (ImGui::IsMouseReleased(0)) {
            // Попробуем найти порт под мышью для завершения соединения
            for (auto& vis : port_visuals_) {
                float dx2 = mouse_pos.x - vis.pos.x;
                float dy2 = mouse_pos.y - vis.pos.y;
                if (dx2 * dx2 + dy2 * dy2 <= hit_radius * hit_radius) {
                    // Не соединяем порт сам с собой
                    if (!(vis.ref.widget_id == drag_from_port_.widget_id &&
                          vis.ref.port == drag_from_port_.port)) {
                        widget_manager_.connections_.push_back({ drag_from_port_, vis.ref });
                    }
                    break;
                }
            }
            is_dragging_connection_ = false;
        }
    }
}

void Editor::RenderConnections(ImDrawList* draw_list)
{
    // Нужно актуальное положение портов из port_visuals_
    //TODO: заменить на map
    auto find_pos = [this](const PortRef& ref) -> ImVec2 {
        for (auto& v : port_visuals_) {                
            if (v.ref.widget_id == ref.widget_id && v.ref.port == ref.port) {
                return v.pos;
            }
        }
        //std::cout <<"NOT FOUND:" << std::endl;
        return ImVec2(0, 0);
    };

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 mouse_pos = io.MousePos;
    const float remove_radius = 8.0f;

    for (int i = (int)widget_manager_.connections_.size() - 1; i >= 0; --i) {
        const auto& c = widget_manager_.connections_[i];
        ImVec2 p1 = find_pos(c.from);
        ImVec2 p2 = find_pos(c.to);

        float dx = (p2.x - p1.x) * 0.5f;
        ImVec2 c1(p1.x + dx, p1.y);
        ImVec2 c2(p2.x - dx, p2.y);
        draw_list->AddBezierCubic(p1, c1, c2, p2,
            IM_COL32(150, 150, 255, 255), 2.0f);

        // Приближённая точка "середины" линии
        ImVec2 mid((p1.x + p2.x) * 0.5f, (p1.y + p2.y) * 0.5f);
        float dxm = mouse_pos.x - mid.x;
        float dym = mouse_pos.y - mid.y;
        if (dxm * dxm + dym * dym <= remove_radius * remove_radius &&
            ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            widget_manager_.connections_.erase(widget_manager_.connections_.begin() + i);
        }
    }
}