#include "EditorMainWindowLayout.h"

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

            if (ImGui::BeginChild("SaveFileChild", ImVec2(400, 80), true)) {

                ImGui::InputText("l", savefile_buffer, IM_ARRAYSIZE(savefile_buffer));
                ImGui::SameLine();
                ImGui::Text(".json");
                ImGui::Dummy(ImVec2(0, 10));
                std::string temp;
                if (ImGui::Button("Save")) {
                    temp = savefile_buffer;
                    auto it = std::find(all_files_.begin(), all_files_.end(), temp);
                    if (it != all_files_.end()) {
                        ImGui::TextColored(ImVec4(1, 0.5f, 0.5f, 1), "Config file with name '%s' already exists", it);
                    }
                    else {
                        SaveConfigWithConnections(std::string(PROJECT_SOURCE_DIR) + "/configs/" + temp + ".json");
                        filebrowser_open_ = false;
                        filesave_open_ = false;
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