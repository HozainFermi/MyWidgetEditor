#include "TableWidget.h"
#include <imgui_internal.h>

namespace wg {

    TableWidget::TableWidget(const std::string& name, const ImVec2& pos) :
        Widget(name, WidgetType::TABLE, pos, ImVec2(400, 300)) {  // Больший размер по умолчанию
        flags_ = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

        // Инициализируем тестовые колонки для превью
        columns_.push_back({ "ID", 50.0f, "id", true });
        columns_.push_back({ "Name", 150.0f, "name", true });
        columns_.push_back({ "Value", 100.0f, "value", false });
    }

    bool TableWidget::UpdateInteraction(const ImVec2& canvas_p0, const ImVec2& canvas_size, int widget_id)
    {
        return Widget::UpdateInteraction(canvas_p0,canvas_size,widget_id);
    }

    void TableWidget::Render(ImDrawList* draw_list, const ImVec2& canvas_p0) {
        Widget::Render(draw_list, canvas_p0);
        
    }

    void TableWidget::RenderContent(ImVec2& screen_min, ImVec2& screen_max) {
        ImVec2 size = ImVec2(screen_max.x - screen_min.x -10,
            screen_max.y - screen_min.y-10);
        ImVec2 pos = ImVec2(screen_min.x+5, screen_min.y+5);
        // Ограничиваем область таблицы
        ImGui::SetNextWindowPos(pos);
        ImGui::SetNextWindowSize(size);

        // Используем окно без заголовка для ограничения области
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(30, 30, 40, 200));
       

        if (ImGui::BeginChild(("##table_child_" + GetId()).c_str(),
            size,
            false, // No border (мы сами рисуем)
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_AlwaysUseWindowPadding)) {

            // Получаем доступную область внутри окна
            ImVec2 content_size = ImGui::GetContentRegionAvail();
           
            // Создаём таблицу
            if (ImGui::BeginTable(("##table_" + GetId()).c_str(),
                (int)columns_.size(),
                flags_,
                content_size)) {
               
                // Устанавливаем заголовки колонок
                for (const auto& col : columns_) {
                    ImGui::TableSetupColumn(col.header.c_str(),
                        col.sortable ? ImGuiTableColumnFlags_None :
                        ImGuiTableColumnFlags_NoSort,
                        col.width);
                }

                if (show_headers_) {
                    ImGui::TableSetupScrollFreeze(0, 1); // Заголовок фиксирован при скролле
                    ImGui::TableHeadersRow();
                }

                static char buf[32];                       
                // Тестовые данные для превью в редакторе
                for (int row = 0; row < 5; row++) {
                    ImGui::TableNextRow();

                    for (int col = 0; col < (int)columns_.size(); col++) {
                        ImGui::TableSetColumnIndex(col);
                        // Тестовые данные
                        if (col == 0) {
                            sprintf_s(buf, "ID_%d", row + 1);
                            ImGui::Selectable(buf, column_selected[col]);
                        }
                        else if (col == 1) {
                            sprintf_s(buf, "Item_%d", row + 1);
                            ImGui::Selectable(buf, column_selected[col]);
                        }
                        else if (col == 2) {
                            sprintf_s(buf, "%.2f", (row + 1) * 10.0f);
                            ImGui::Selectable(buf, column_selected[col]);
                        }
                    }
                }

                ImGui::EndTable();
            }

            ImGui::EndChild();
        }

        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
    }

    void TableWidget::RenderProperties() {
        // Настройки таблицы
        ImGui::Text("Table Settings");
        ImGui::Separator();

        ImGui::Checkbox("Show Headers", &show_headers_);
        ImGui::Checkbox("Alternate Row Colors", &alternate_row_colors_);
        ImGui::Checkbox("Show Borders", &show_borders_);

        if (show_borders_) {
            flags_ |= ImGuiTableFlags_Borders;
        }
        else {
            flags_ &= ~ImGuiTableFlags_Borders;
        }

        if (alternate_row_colors_) {
            flags_ |= ImGuiTableFlags_RowBg;
        }
        else {
            flags_ &= ~ImGuiTableFlags_RowBg;
        }

        ImGui::Separator();

        // Настройки колонок
        ImGui::Text("Columns:");
        for (size_t i = 0; i < columns_.size(); i++) {
            ImGui::PushID((int)i);

            ImGui::Text("Column %d:", (int)i + 1);
            ImGui::SameLine();

            char header_buf[128];
            strncpy_s(header_buf, columns_[i].header.c_str(), sizeof(header_buf));
            ImGui::SetNextItemWidth(50);
            if (ImGui::InputText("##header", header_buf, sizeof(header_buf))) {
                columns_[i].header = header_buf;
            }
            
            ImGui::SameLine();
            ImGui::SetNextItemWidth(50);
            ImGui::DragFloat("##width", &columns_[i].width, 1.0f, 20.0f, 500.0f);

            ImGui::SameLine();
            if (ImGui::Button("Remove")) {
                RemoveColumn((int)i);
                i--; // Уменьшаем индекс после удаления
            }

            ImGui::PopID();
        }

        if (ImGui::Button("Add Column")) {
            AddColumn("New Column", 100.0f);
        }

        ImGui::Separator();

        // Настройки источника данных
        ImGui::Text("Data Source:");
        const char* source_types[] = { "None", "CSV URL", "JSON URL", "CSV File", "JSON File", "Static Data" };
        int current_type = (int)data_source_type_;
        if (ImGui::Combo("Type", &current_type, source_types, IM_ARRAYSIZE(source_types))) {
            //Widget::stay_selected_ = true;
            data_source_type_ = (DataSourceType)current_type;
        }

        if (data_source_type_ != DataSourceType::NONE) {
            char source_buf[256];
            strncpy_s(source_buf, data_source_.c_str(), sizeof(source_buf));
            if (ImGui::InputText("Source", source_buf, sizeof(source_buf))) {
                data_source_ = source_buf;
            }

            // Настройки обновления
            const char* triggers[] = { "None", "Timer", "Button Click", "On Load" };
            int current_trigger = (int)update_trigger_;
            if (ImGui::Combo("Update Trigger", &current_trigger, triggers, IM_ARRAYSIZE(triggers))) {
                update_trigger_ = (UpdateTrigger)current_trigger;
            }

            if (update_trigger_ == UpdateTrigger::TIMER) {
                ImGui::DragFloat("Interval (s)", &update_interval_, 0.1f, 0.1f, 60.0f);
            }
        }
    }

    void TableWidget::AddColumn(const std::string& header, float width, const std::string& data_field) {
        columns_.push_back({ header, width, data_field, false });
    }

    void TableWidget::RemoveColumn(int index) {
        if (index >= 0 && index < (int)columns_.size()) {
            columns_.erase(columns_.begin() + index);
        }
    }

    void TableWidget::SetDataSource(const std::string& source, DataSourceType type) {
        data_source_ = source;
        data_source_type_ = type;
    }

    void TableWidget::SetUpdateTrigger(UpdateTrigger trigger, float interval) {
        update_trigger_ = trigger;
        update_interval_ = interval;
    }

    void TableWidget::SetStaticData(const std::vector<std::vector<std::string>>& data) {
        static_data_ = data;
    }

    nlohmann::json TableWidget::ToJson() const {
        auto json = Widget::ToJson();

        // Колонки
        nlohmann::json columns_json = nlohmann::json::array();
        for (const auto& col : columns_) {
            columns_json.push_back(col.ToJson());
        }
        json["columns"] = columns_json;

        // Источник данных
        json["data_source_type"] = (int)data_source_type_;
        json["data_source"] = data_source_;
        json["update_trigger"] = (int)update_trigger_;
        json["update_interval"] = update_interval_;

        // Настройки таблицы
        json["flags"] = flags_;
        json["show_headers"] = show_headers_;
        json["show_borders"] = show_borders_;
        json["alternate_row_colors"] = alternate_row_colors_;
        json["max_display_rows"] = max_display_rows_;

        // Статические данные (если есть)
        if (!static_data_.empty()) {
            json["static_data"] = static_data_;
        }

        // HTTP настройки
        if (!api_key_.empty()) {
            json["api_key"] = api_key_;
        }
        if (!custom_headers_.empty()) {
            json["custom_headers"] = custom_headers_;
        }

        return json;
    }

    void TableWidget::FromJson(const nlohmann::json& json) {
        Widget::FromJson(json);

        // Колонки
        if (json.contains("columns") && json["columns"].is_array()) {
            columns_.clear();
            for (const auto& col_json : json["columns"]) {
                TableColumnConfig col;
                col.FromJson(col_json);
                columns_.push_back(col);
            }
        }

        // Источник данных
        if (json.contains("data_source_type")) {
            data_source_type_ = (DataSourceType)json["data_source_type"].get<int>();
        }
        if (json.contains("data_source")) {
            data_source_ = json["data_source"].get<std::string>();
        }
        if (json.contains("update_trigger")) {
            update_trigger_ = (UpdateTrigger)json["update_trigger"].get<int>();
        }
        if (json.contains("update_interval")) {
            update_interval_ = json["update_interval"].get<float>();
        }

        // Настройки таблицы
        if (json.contains("flags")) {
            flags_ = json["flags"].get<int>();
        }
        if (json.contains("show_headers")) {
            show_headers_ = json["show_headers"].get<bool>();
        }
        if (json.contains("show_borders")) {
            show_borders_ = json["show_borders"].get<bool>();
        }
        if (json.contains("alternate_row_colors")) {
            alternate_row_colors_ = json["alternate_row_colors"].get<bool>();
        }
        if (json.contains("max_display_rows")) {
            max_display_rows_ = json["max_display_rows"].get<int>();
        }

        // Статические данные
        if (json.contains("static_data") && json["static_data"].is_array()) {
            static_data_.clear();
            for (const auto& row_json : json["static_data"]) {
                std::vector<std::string> row;
                for (const auto& cell_json : row_json) {
                    row.push_back(cell_json.get<std::string>());
                }
                static_data_.push_back(row);
            }
        }

        // HTTP настройки
        if (json.contains("api_key")) {
            api_key_ = json["api_key"].get<std::string>();
        }
        if (json.contains("custom_headers")) {
            custom_headers_ = json["custom_headers"].get<std::map<std::string, std::string>>();
        }
    }

    void TableWidget::GeneratePreviewData() {
        // Генерируем тестовые данные для превью
        static_data_.clear();
        for (int i = 0; i < 5; i++) {
            std::vector<std::string> row;
            for (size_t j = 0; j < columns_.size(); j++) {
                if (j == 0) {
                    row.push_back("ID_" + std::to_string(i + 1));
                }
                else if (j == 1) {
                    row.push_back("Item_" + std::to_string(i + 1));
                }
                else {
                    row.push_back("Value_" + std::to_string(i + 1));
                }
            }
            static_data_.push_back(row);
        }
    }
}