#include "EditorMainWindowLayout.h"
#include <iostream>
#include <core/WidgetFactory.h>
#include <core/Config.h>
#include "independent_launcher.h"
#include <filesystem>
#include <fstream>
#include <IconText.h>
#include <unordered_map>


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
    QueueAction(PendingAction::Load, filename, "Loading...");
}

void Editor::OnFileForRunSelected(const std::string& filename) {
    QueueAction(PendingAction::Run, filename, "Launching...");
}

void Editor::SaveConfigWithConnections(const std::string& filename) {
    QueueAction(PendingAction::Save, filename, "Saving...");
}

void Editor::SaveCurrentConfig() {
    if (loaded_config.empty()) {
        SetStatus("Nothing to save into: use File > New", true);
        return;
    }
    SaveConfigWithConnections(std::string(PROJECT_SOURCE_DIR) + "/configs/" + loaded_config + ".json");
}

bool Editor::ExecuteLoad(const std::string& filename) {
    if (!widget_manager_.LoadFromFile(std::string(PROJECT_SOURCE_DIR) + "/configs/" + filename + ".json", window_props_)) {
        SetStatus("Can't load " + filename, true);
        return false;
    }
    loaded_config = filename;
    SetStatus("Loaded " + filename, false);
    return true;
}

bool Editor::ExecuteRun(const std::string& filename) {
    std::filesystem::path relfilePath(std::string(PROJECT_SOURCE_DIR)+"/configs/" + filename + ".json");
    std::filesystem::path relDrawer("../Drawer/Drawer.exe");
    //C:\Users\Hoz\Desktop\MyWidgetEditor\out\build\x64-debug\Drawer
    std::filesystem::path absExePath = std::filesystem::absolute(relDrawer);
    std::filesystem::path absConfigPath = std::filesystem::absolute(relfilePath);

    // Проверка существования файлов
    if (!std::filesystem::exists(absExePath)) {
        std::cerr << "Error: Executable not found: " << absExePath << "\n";
        SetStatus("Drawer.exe not found", true);
        return false;
    }

    if (!std::filesystem::exists(absConfigPath)) {
        std::cerr << "Error: Config file not found: " << absConfigPath << "\n";
        SetStatus("Config not found: " + filename, true);
        return false;
    }

    std::string exePath = "\"" + absExePath.string() + "\"";
    std::string configPath = "\"" + absConfigPath.string() + "\""; 

    std::cout << "Executing: " << exePath << " " << configPath << std::endl;

    //Раздельная передача
#ifdef _WIN32   
    HANDLE proc = IndependentLauncher::launch(exePath, configPath);
    if (proc == nullptr) {
        std::cerr << "Failed to launch process\n";
        SetStatus("Can't start the widget", true);
        return false;
    }
    Process process(proc, filename);
    launching_pid_ = process.GetPid();
    launch_deadline_ = ImGui::GetTime() + 10.0;
    processes_.Add(std::move(process));
    // Спиннер крутится, пока у виджета не появится окно (см. RenderStatus)
    status_text_ = "Starting " + filename + "...";
    status_busy_ = true;
    return true;
#else
    pid_t pid = IndependentLauncher::launch(exePath, configPath);
    if (pid == 0) {
        std::cerr << "Failed to launch process\n";
        SetStatus("Can't start the widget", true);
        return false;
    }
    processes_.Add(Process(pid, filename));
    SetStatus("Started " + filename, false);
    return true;
#endif

}


bool Editor::ExecuteSave(const std::string& path)
{
    std::string error;
    if (!core::SaveConfig(path, widget_manager_.ToJson(window_props_), &error)) {
        std::cerr << error << std::endl;
        SetStatus("Can't save: " + error, true);
        return false;
    }
    SetStatus("Saved " + std::filesystem::path(path).stem().string(), false);
    return true;
}

// ===== Статус операций =====

namespace {
#ifdef _WIN32
    struct WindowSearch { DWORD pid; bool found; };

    BOOL CALLBACK FindProcessWindow(HWND hwnd, LPARAM param) {
        auto* search = reinterpret_cast<WindowSearch*>(param);
        DWORD pid = 0;
        GetWindowThreadProcessId(hwnd, &pid);
        if (pid == search->pid && IsWindowVisible(hwnd)) {
            search->found = true;
            return FALSE;
        }
        // Окно виджета в режиме обоев - дочернее окно WorkerW, ищем и среди дочерних
        EnumChildWindows(hwnd, FindProcessWindow, param);
        return search->found ? FALSE : TRUE;
    }

    bool ProcessHasWindow(uint32_t pid) {
        WindowSearch search{ pid, false };
        EnumWindows(FindProcessWindow, reinterpret_cast<LPARAM>(&search));
        return search.found;
    }
#endif

    // Крутящаяся дуга размером со строку текста
    void Spinner(ImU32 color) {
        const float size = ImGui::GetTextLineHeight();
        const ImVec2 pos = ImGui::GetCursorScreenPos();
        ImGui::Dummy(ImVec2(size, size));
        const ImVec2 center(pos.x + size * 0.5f, pos.y + size * 0.5f);
        const float start = (float)ImGui::GetTime() * 6.0f;
        ImDrawList* dl = ImGui::GetWindowDrawList();
        dl->PathArcTo(center, size * 0.4f, start, start + IM_PI * 1.4f, 20);
        dl->PathStroke(color, 0, 2.0f);
    }

    // Галочка (ok) или крестик (ошибка)
    void ResultMark(bool error) {
        const float size = ImGui::GetTextLineHeight();
        const ImVec2 p = ImGui::GetCursorScreenPos();
        ImGui::Dummy(ImVec2(size, size));
        ImDrawList* dl = ImGui::GetWindowDrawList();
        if (error) {
            const ImU32 red = IM_COL32(240, 90, 90, 255);
            dl->AddLine(ImVec2(p.x + size * 0.2f, p.y + size * 0.2f), ImVec2(p.x + size * 0.8f, p.y + size * 0.8f), red, 2.0f);
            dl->AddLine(ImVec2(p.x + size * 0.8f, p.y + size * 0.2f), ImVec2(p.x + size * 0.2f, p.y + size * 0.8f), red, 2.0f);
        }
        else {
            ImVec2 check[] = { ImVec2(p.x + size * 0.15f, p.y + size * 0.55f), ImVec2(p.x + size * 0.4f, p.y + size * 0.8f), ImVec2(p.x + size * 0.85f, p.y + size * 0.25f) };
            dl->AddPolyline(check, 3, IM_COL32(110, 210, 120, 255), 0, 2.0f);
        }
    }
}

void Editor::QueueAction(PendingAction action, const std::string& arg, const char* busy_text) {
    pending_action_ = action;
    pending_arg_ = arg;
    pending_frame_shown_ = false;
    status_text_ = busy_text;
    status_busy_ = true;
    status_error_ = false;
}

void Editor::RunPendingAction() {
    if (pending_action_ == PendingAction::None) {
        return;
    }
    // Сначала кадр со спиннером, потом сама (блокирующая) операция
    if (!pending_frame_shown_) {
        pending_frame_shown_ = true;
        return;
    }
    PendingAction action = pending_action_;
    pending_action_ = PendingAction::None;
    switch (action) {
    case PendingAction::Save: ExecuteSave(pending_arg_); break;
    case PendingAction::Load: ExecuteLoad(pending_arg_); break;
    case PendingAction::Run:  ExecuteRun(pending_arg_); break;
    default: break;
    }
}

void Editor::SetStatus(const std::string& text, bool error) {
    status_text_ = text;
    status_busy_ = false;
    status_error_ = error;
    status_until_ = ImGui::GetTime() + (error ? 6.0 : 2.5);
}

void Editor::RenderStatus() {
#ifdef _WIN32
    // Запуск закончен, когда у виджета появилось окно (или процесс сразу умер)
    if (launching_pid_ != 0 && pending_action_ == PendingAction::None) {
        HANDLE process = OpenProcess(SYNCHRONIZE, FALSE, launching_pid_);
        const bool exited = !process || WaitForSingleObject(process, 0) == WAIT_OBJECT_0;
        if (process) CloseHandle(process);

        if (exited) {
            SetStatus("Widget exited right after start", true);
            launching_pid_ = 0;
        }
        else if (ProcessHasWindow(launching_pid_)) {
            SetStatus("Widget started", false);
            launching_pid_ = 0;
        }
        else if (ImGui::GetTime() > launch_deadline_) {
            SetStatus("Widget started, but no window yet", true);
            launching_pid_ = 0;
        }
    }
#endif

    if (status_busy_) {
        Spinner(ImGui::GetColorU32(ImGuiCol_Text));
        ImGui::SameLine();
        ImGui::TextUnformatted(status_text_.c_str());
    }
    else if (!status_text_.empty() && ImGui::GetTime() < status_until_) {
        ResultMark(status_error_);
        ImGui::SameLine();
        if (status_error_) {
            ImGui::TextColored(ImVec4(1.0f, 0.45f, 0.45f, 1.0f), "%s", status_text_.c_str());
        }
        else {
            ImGui::TextUnformatted(status_text_.c_str());
        }
    }
    else {
        ImGui::NewLine();
    }
}

void Editor::CheckFileMenuHotKeys() {
    ImGuiIO& io = ImGui::GetIO();
    // Не перехватываем Ctrl+буква, пока пользователь печатает в поле ввода
    if (!io.KeyCtrl || io.WantTextInput) {
        return;
    }
    if (ImGui::IsKeyPressed(ImGuiKey_N, false)) {
        filesave_open_ = true;
    }
    if (ImGui::IsKeyPressed(ImGuiKey_S, false)) {
        SaveCurrentConfig();
    }
    if (ImGui::IsKeyPressed(ImGuiKey_L, false)) {
        filebrowser_open_ = true;
        browsermode = FileBrowserMode::Load;
    }
}

// Реализация методов класса Editor
void Editor::RenderMenuBar() {

    CheckFileMenuHotKeys();
        
    if (ImGui::BeginMenuBar()) {        
        if (ImGui::BeginMenu("File ")) {
            if (ImGui::MenuItem("New", "Ctrl+N")) {                
                filesave_open_ = true;                
            }
            if (ImGui::MenuItem("Run", "")) {          
                filebrowser_open_ = true;
                browsermode = FileBrowserMode::Run;
            }
            if (ImGui::MenuItem("Save ", "Ctrl+S")) {                
               SaveCurrentConfig();                
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
        processes_.RemoveExited();  // виджеты, которые закрылись сами
        char processes_label[32];
        snprintf(processes_label, sizeof(processes_label), "Processes (%zu)###processes", processes_.size());  // ### - ID меню не меняется вместе с числом
        if (ImGui::BeginMenu(processes_label)) {
            if (ImGui::MenuItem("Refresh Desktop Wallpaper", NULL)) {
                IndependentLauncher::RefreshDesktopWallpaperLayer();
            }
            if (ImGui::MenuItem("Kill All", NULL, false, !processes_.empty())) {
                processes_.KillAll();
                IndependentLauncher::RefreshDesktopWallpaperLayer();  // убрать след виджета с обоев
            }
            ImGui::Separator();
            if (processes_.empty()) {
                ImGui::TextDisabled("No running widgets");
            }
            for (size_t i = 0; i < processes_.size(); ++i) {
                char label[160];
                snprintf(label, sizeof(label), "%s (pid %u)##%zu", processes_[i].GetName().c_str(), processes_[i].GetPid(), i);
                if (ImGui::BeginMenu(label)) {
                    if (ImGui::MenuItem("Kill Process")) {
                        processes_.Kill(i);
                        IndependentLauncher::RefreshDesktopWallpaperLayer();
                        ImGui::EndMenu();
                        break;
                    }
                    ImGui::EndMenu();
                }
            }
            ImGui::TextDisabled("Widgets keep running after the editor closes");
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
    ImGui::Text("Logic Nodes");
    ImGui::SameLine();
    HelpMarker("Drag to the Logic tab");
    ImGui::Separator();
    static const std::vector<std::string> logic_nodes = core::WidgetFactory::GetRegisteredTypes(core::WidgetCategory::Logic);
    for (const auto& node : logic_nodes) {
        ImGui::Selectable(node.c_str());
        if (ImGui::BeginDragDropSource(ImGuiDragDropFlags_SourceNoDisableHover)) {
            ImGui::SetDragDropPayload("LOGIC_NODE", node.c_str(), node.size() + 1);
            ImGui::Text("Add %s", node.c_str());
            ImGui::EndDragDropSource();
            canvas_mode_ = CanvasMode::Logic;  // узлы живут только на вкладке Logic
        }
    }

    ImGui::Separator();
    ImGui::Text("Used Widgets");
    ImGui::SameLine();
    HelpMarker("Click to select");
    ImGui::Separator();
    for (core::Widget* widget : widget_manager_.GetAllWidgets()) {
        if (ImGui::Selectable(widget->GetId().c_str(), widget_manager_.IsSelected(widget))) {
            widget_manager_.Select(widget->GetId());
        }
    }
}

void Editor::RenderRightPanel() {
    core::Widget* selected = widget_manager_.GetSelectedWidget();

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

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.65f);
    if (ImGui::ColorEdit4("MainWindowColor", window_props_.bg_color_float, ImGuiColorEditFlags_DisplayHSV)) {

        window_props_.bg_color = ImGui::ColorConvertFloat4ToU32(
            ImVec4(window_props_.bg_color_float[0],
                window_props_.bg_color_float[1],
                window_props_.bg_color_float[2],
                window_props_.bg_color_float[3]));
    }

    // ===== Режим окна =====
    static const char* kModes[] = { "Normal", "Wallpaper", "HUD" };
    int mode = (int)window_props_.mode;
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.65f);
    if (ImGui::Combo("Mode", &mode, kModes, IM_ARRAYSIZE(kModes))) {
        window_props_.mode = (core::WindowMode)mode;
        window_props_.ApplyMode();
    }
    ImGui::SameLine();
    HelpMarker("Normal: a regular window, set the properties yourself.\n"
               "Wallpaper: on the desktop under the icons, full screen.\n"
               "HUD: on top of everything, full screen, clicks go through, background is transparent.\n\n"
               "Emergency close of running widgets: Ctrl+Alt+Shift+Q.");
    if (window_props_.mode == core::WindowMode::Hud) {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.35f, 1.0f), "HUD: background and shader are transparent.\nClose: Ctrl+Alt+Shift+Q");
    }

    // ===== Свойства окна (флаги, заданные режимом, не меняются) =====
    struct WindowFlag { const char* name; bool* value; bool locked_by_mode; };
    const bool normal_mode = window_props_.mode == core::WindowMode::Normal;
    WindowFlag flags[] = {
        { "Full screen",        &window_props_.full_screen,        !normal_mode },
        { "Always on top",      &window_props_.always_on_top,      !normal_mode },
        { "Always on bottom",   &window_props_.always_on_bottom,   !normal_mode },
        { "Mouse passthrough",  &window_props_.mouse_passthrougth, !normal_mode },
        { "Window rounding",    &window_props_.window_rounding,    !normal_mode },
        { "Resizable",          &window_props_.resizeble,          !normal_mode },
        { "Movable",            &window_props_.moveble,            !normal_mode },
        { "Decorated",          &window_props_.decorated,          !normal_mode },
    };
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.65f);
    if (ImGui::BeginCombo("##window_properties", "Window properties"))
    {
        if (!normal_mode) {
            ImGui::TextDisabled("Set by the \"%s\" mode", kModes[mode]);
        }
        for (auto& flag : flags) {
            ImGui::BeginDisabled(flag.locked_by_mode);
            if (ImGui::Checkbox(flag.name, flag.value)) {
                // Поверх всех и под всеми одновременно - нельзя
                if (flag.value == &window_props_.always_on_top && window_props_.always_on_top) window_props_.always_on_bottom = false;
                if (flag.value == &window_props_.always_on_bottom && window_props_.always_on_bottom) window_props_.always_on_top = false;
            }
            ImGui::EndDisabled();
        }
        ImGui::EndCombo();
    }
    if (normal_mode && window_props_.IsClickThroughOverlay()) {
        ImGui::TextColored(ImVec4(1.0f, 0.8f, 0.35f, 1.0f), "On top + full screen + passthrough:\nbackground will be transparent.");
    }

    if (window_props_.window_rounding) {
        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.65f);
        ImGui::DragFloat("Rounding",&window_props_.rounding,0.5f,0.0f,500.0f);
    }

    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.65f);
    ImGui::DragInt("FPS limit", &window_props_.fps_limit, 1, 0, 240);
    ImGui::SameLine();
    HelpMarker("Max frames per second of the running widget (0 = no limit, vsync).\n"
               "Without user input it redraws only as often as its widgets need:\n"
               "static widgets ~1-2 FPS, animations ~30. A shader background always uses the limit.");

    if (ImGui::Button(ICON(ICON_FOLDER), ImVec2(30,22)) ) {
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
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.618f);
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
        // Присваивание unique_ptr само удалит старый шейдер (release() его терял)
        background_scene_.models_[0].shader = std::move(shaders);
    }
    //if (ImGui::InputText("Vert Shader", VERTbuf, IM_ARRAYSIZE(VERTbuf))) {
    //    window_props_.vertex_GLSLshader_file = VERTbuf;
    //}
    ImGui::Separator();

    if (selected) {
        
        ImGui::Text("Properties: %s", selected->GetName().c_str());
        ImGui::Separator();

        if (!selected->IsLogicNode()) {
            ImVec2 pos = selected->GetPosition();
            if (ImGui::DragFloat2("Position", &pos.x, 1.0f)) {
                selected->SetPosition(pos);
            }
            ImVec2 size = selected->GetSize();
            if (ImGui::DragFloat2("Size", &size.x, 1.0f, 10.0f, 1000.0f)) {
                selected->SetSize(size);
            }
        }

        // Имя
        static char name_buffer[256];
        strcpy_s(name_buffer, selected->GetName().c_str());
        if (ImGui::InputText("Name", name_buffer, sizeof(name_buffer))) {
            selected->SetName(name_buffer);
        }

        // Тип виджета (только для чтения)
        ImGui::Text("Type: %s", core::WidgetTypeToString(selected->GetType()));

        // Общие настройки для запущенного виджета (Drawer); узлы логики не рисуются вовсе
        if (!selected->IsLogicNode()) {
            bool visible = selected->IsVisible();
            if (ImGui::Checkbox("Visible in widget", &visible)) {
                selected->SetVisible(visible);
            }
            ImGui::SameLine();
            HelpMarker("Hidden widgets are not drawn in the running widget but keep working\n(e.g. a table that only feeds data to a plot).");

            bool fade = selected->GetFadeOnHover();
            if (ImGui::Checkbox("Fade on hover", &fade)) {
                selected->SetFadeOnHover(fade);
            }
            ImGui::SameLine();
            HelpMarker("In the running widget: while the mouse is over the widget it gradually becomes transparent,\nwhen the mouse leaves it comes back.");
            if (fade) {
                float min_opacity = selected->GetFadeMinAlpha() * 100.0f;
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
                if (ImGui::SliderFloat("Min opacity", &min_opacity, 0.0f, 100.0f, "%.0f%%")) {
                    selected->SetFadeMinAlpha(min_opacity / 100.0f);
                }
                float fade_time = 1.0f / selected->GetFadeSpeed();
                ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
                if (ImGui::SliderFloat("Fade time", &fade_time, 0.1f, 5.0f, "%.1f s")) {
                    selected->SetFadeSpeed(1.0f / fade_time);
            }
        }
        }
        ImGui::Separator();

        // Кнопки действий
        bool delete_pressed = ImGui::Button("Delete", ImVec2(-1, 0));

        if (ImGui::Button("Duplicate", ImVec2(-1, 0))) {
            // Логика дублирования
        }

        if (ImGui::Button("Bring to Front", ImVec2(-1, 0))) {
            widget_manager_.BringToFront(selected->GetId());
        }
        ImGui::Separator();
        selected->RenderProperties();

        // Удаляем в конце: дальше по коду selected уже не нужен
        if (delete_pressed) {
            widget_manager_.DeleteWidget(selected->GetId());
            selected = nullptr;
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

    std::unique_ptr<core::Widget> widget = core::WidgetFactory::Create(widget_class);
    if (!widget) {
        std::cerr << "Widget class not registered: " << widget_class << std::endl;
        return;
    }
    widget->SetName(widget_class + std::to_string(widget_counter++));
    if (widget->IsLogicNode()) {
        widget->SetGraphPosition(position);
    }
    else {
        widget->SetPosition(position);
    }
    widget_manager_.AddWidget(std::move(widget));
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
    RunPendingAction();

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
            // Ширина канваса сохраняется в imgui.ini. Если окно стало меньше, без ограничения
            // правая панель уезжает за край, поэтому оставляем ей минимум места
            const float right_panel_min_width = 250.0f;
            const float canvas_max_width = ImMax(200.0f, ImGui::GetContentRegionAvail().x - right_panel_min_width);
            ImGui::SetNextWindowSizeConstraints(ImVec2(200.0f, 0.0f), ImVec2(canvas_max_width, FLT_MAX));
            ImGui::BeginChild("CanvasPanel", ImVec2(ImGui::GetContentRegionAvail().x * 0.6f, 0),
                ImGuiChildFlags_ResizeX |ImGuiChildFlags_ResizeY | ImGuiChildFlags_Borders);
            RenderCanvas();
            ImGui::EndChild();

            ImGui::SameLine();
            
            // Правая панель (20%)
            ImGui::BeginChild("RightPanel", ImVec2(0, 0), ImGuiChildFlags_Borders);
            RenderRightPanel();
            widget_manager_.SetKeepSelection(ImGui::IsWindowHovered(ImGuiHoveredFlags_ChildWindows));
            ImGui::EndChild();            
        }
        ImGui::EndChild();
    }
    ImGui::End();
}


// Реализация RenderCanvas 
void Editor::RenderCanvas() {
    
    ImGui::Text("Config: %s", loaded_config.empty() ? "(not saved)" : loaded_config.c_str());
    ImGui::SameLine(0.0f, 20.0f);
    RenderStatus();
    if (ImGui::RadioButton("Layout", canvas_mode_ == CanvasMode::Layout)) canvas_mode_ = CanvasMode::Layout;
    ImGui::SameLine();
    if (ImGui::RadioButton("Logic", canvas_mode_ == CanvasMode::Logic)) canvas_mode_ = CanvasMode::Logic;
    ImGui::SameLine();
    HelpMarker("Layout: where widgets are in the window.\nLogic: how data flows between widgets and logic nodes (Filter, Map...).");
    ImGui::SameLine(0.0f, 20.0f);
    ImGui::TextDisabled("Ctrl+S to save, Ctrl+L to load");
    ImGui::Text("Grid size");
    
    ImGui::Checkbox("Grid", &show_grid_);
    ImGui::SameLine();
    ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.5f);
    ImGui::DragFloat(" ", &grid_size_, 1.0f, 5.0f, 100.0f);
    ImGui::SameLine();
    bool live_data = widget_manager_.IsLiveData();
    if (ImGui::Checkbox("Live data", &live_data)) {
        widget_manager_.SetLiveData(live_data);
    }
    ImGui::SameLine();
    HelpMarker("Off: widgets show preview data.\nOn: widgets load real data (HTTP requests etc.) like in the running widget.");

    if (canvas_mode_ == CanvasMode::Logic) {
        RenderLogicCanvas();
        return;
    }

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

        background_scene_.ResizeFramebuffer(background_scene_.SCR_WIDTH, background_scene_.SCR_HEIGHT);
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
        if (!w || w->IsLogicNode()) continue;
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
                    // Соединяем только выход со входом разных виджетов.
                    // Если тянули от входа к выходу - разворачиваем связь.
                    const PortVisual* from_vis = nullptr;
                    for (const auto& candidate : port_visuals_) {
                        if (SamePort(candidate.ref, drag_from_port_)) {
                            from_vis = &candidate;
                            break;
                        }
                    }
                    if (from_vis && from_vis->is_input != vis.is_input &&
                        vis.ref.widget_id != drag_from_port_.widget_id) {
                        Connection connection = from_vis->is_input
                            ? Connection{ vis.ref, drag_from_port_ }
                            : Connection{ drag_from_port_, vis.ref };
                        widget_manager_.AddConnection(connection);
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
    // Актуальные позиции портов из port_visuals_ (map вместо линейного поиска)
    std::unordered_map<std::string, ImVec2> port_positions;
    port_positions.reserve(port_visuals_.size());
    for (const auto& v : port_visuals_) {
        port_positions[PortKey(v.ref)] = v.pos;
    }
    auto find_pos = [&port_positions](const PortRef& ref) -> ImVec2 {
        auto it = port_positions.find(PortKey(ref));
        return it != port_positions.end() ? it->second : ImVec2(0, 0);
    };

    ImGuiIO& io = ImGui::GetIO();
    ImVec2 mouse_pos = io.MousePos;
    const float remove_radius = 8.0f;

    const auto& connections = widget_manager_.GetConnections();
    for (int i = (int)connections.size() - 1; i >= 0; --i) {
        const auto& c = connections[i];
        // Связи с узлами логики видны только на вкладке Logic
        if (!port_positions.count(PortKey(c.from)) || !port_positions.count(PortKey(c.to))) continue;
        ImVec2 p1 = find_pos(c.from);
        ImVec2 p2 = find_pos(c.to);

        float dx = (p2.x - p1.x) * 0.5f;
        ImVec2 c1(p1.x + dx, p1.y);
        ImVec2 c2(p2.x - dx, p2.y);
        draw_list->AddBezierCubic(p1, c1, c2, p2,
            IM_COL32(150, 150, 255, 255), 2.0f);

        // Приближённая точка середины линии
        ImVec2 mid((p1.x + p2.x) * 0.5f, (p1.y + p2.y) * 0.5f);
        float dxm = mouse_pos.x - mid.x;
        float dym = mouse_pos.y - mid.y;
        if (dxm * dxm + dym * dym <= remove_radius * remove_radius &&
            ImGui::IsMouseClicked(ImGuiMouseButton_Right)) {
            widget_manager_.RemoveConnection(i);
        }
    }
}