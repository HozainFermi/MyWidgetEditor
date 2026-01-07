#include "TableWidget.h"
#include <imgui_internal.h>


namespace rn {

    REGISTER_WIDGET(TableWidget);

    TableWidget::TableWidget() :
        Widget("Unnamed", WidgetType::TABLE, ImVec2(20, 20), ImVec2(400, 300)) {  // Больший размер по умолчанию
        flags_ = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

        // Инициализируем тестовые колонки для превью
        columns_.push_back({ "ID", "id",50.0f, true });
        columns_.push_back({ "Name", "name", 150.0f,true });
        columns_.push_back({ "Value", "value",100.0f, false });

        SetWidgetClass("TableWidget");
    }

    TableWidget::TableWidget(const std::string& name, const ImVec2& pos) :
        Widget(name, WidgetType::TABLE, pos, ImVec2(400, 300)) {  // Больший размер по умолчанию
        flags_ = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

        // Инициализируем тестовые колонки для превью
        columns_.push_back({ "ID", "id", 50.0f,true });
        columns_.push_back({ "Name", "name", 150.0f,true });
        columns_.push_back({ "Value", "value",100.0f, false });

        SetWidgetClass("TableWidget");
    }

    bool TableWidget::UpdateInteraction(int widget_id)
    {
        return Widget::UpdateInteraction(widget_id);
    }

    void TableWidget::Render(ImDrawList* draw_list) {
        Widget::Render(draw_list);

    }

    void TableWidget::RenderContent(ImVec2& screen_min, ImVec2& screen_max) {
        ImVec2 size = ImVec2(screen_max.x - screen_min.x - 10,
            screen_max.y - screen_min.y - 10);
        ImVec2 pos = ImVec2(screen_min.x + 5, screen_min.y + 5);
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

   

    void TableWidget::AddColumn(const std::string& header, float width, const std::string& data_field) {
        columns_.push_back({ header, data_field,width, false });
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

    void TableWidget::GenerateData() {
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