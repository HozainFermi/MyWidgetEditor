#include "TableWidget.h"
#include <iostream>
#include <stdexcept>
#include <imgui_internal.h>
#include <imgui_stdlib.h>
#include "core/Http.h"

namespace core {

    namespace {
        // Ответ должен быть массивом JSON-объектов
        std::vector<WidgetValue> ParseJsonRows(const std::string& body) {
            nlohmann::json json = nlohmann::json::parse(body);
            if (!json.is_array()) {
                throw std::invalid_argument("Response body is not an array");
            }
            std::vector<WidgetValue> rows;
            rows.reserve(json.size());
            for (auto& row : json) {
                if (!row.is_object()) {
                    throw std::invalid_argument("Element in array is not an object");
                }
                rows.push_back(std::move(row));
            }
            return rows;
        }

        std::string CellToString(const nlohmann::json& value) {
            return value.is_string() ? value.get<std::string>() : value.dump();
        }
    }

    TableWidget::TableWidget()
        : Widget(WidgetType::TABLE, ImVec2(400, 300)) {
        flags_ = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg | ImGuiTableFlags_SizingStretchProp | ImGuiTableFlags_Resizable;

        // Колонки для превью в редакторе
        columns_.push_back(TableColumnConfig{ "ID", "id", 50, true });
        columns_.push_back(TableColumnConfig{ "Name", "name", 150, true });
        columns_.push_back(TableColumnConfig{ "Value", "value", 100, false });
    }

    bool TableWidget::IsUrlSource() const {
        return data_source_type_ == DataSourceType::JSON_URL;
    }

    void TableWidget::OnContextChanged() {
        // После переключения на живые данные грузим заново
        loaded_once_ = false;
    }

    void TableWidget::OnPortsConnected() {
        // Новый подписчик сразу получает уже загруженные данные
        if (!ShowsPreview() && !live_rows_.empty()) {
            Emit("data", live_rows_);
        }
    }

    void TableWidget::Update() {
        if (ShowsPreview()) {
            return;
        }

        // Забираем результат фоновой загрузки
        std::vector<WidgetValue> rows;
        bool got_rows = false;
        {
            std::lock_guard<std::mutex> lock(pending_mutex_);
            if (has_pending_) {
                rows = std::move(pending_rows_);
                has_pending_ = false;
                got_rows = true;
            }
        }
        if (got_rows) {
            ApplyRows(std::move(rows));
        }

        if (data_source_type_ == DataSourceType::STATIC_DATA) {
            if (!loaded_once_) {
                ApplyStaticData();
            }
            return;
        }

        if (!IsUrlSource() || is_loading_) {
            return;
        }

        const auto now = std::chrono::steady_clock::now();
        switch (update_trigger_) {
        case UpdateTrigger::ON_LOAD:
            if (!loaded_once_) {
                StartLoading();
            }
            break;
        case UpdateTrigger::TIMER:
            if (!loaded_once_ || now - last_update_ >= std::chrono::seconds(update_interval_)) {
                StartLoading();
            }
            break;
        default:
            break;
        }
    }

    void TableWidget::StartLoading() {
        loaded_once_ = true;
        last_update_ = std::chrono::steady_clock::now();
        is_loading_ = true;

        if (load_thread_.joinable()) {
            load_thread_.join();  // предыдущая загрузка уже закончилась (is_loading_ был false)
        }

        // Поток получает копии настроек и не трогает остальные поля виджета
        std::string url = data_source_;
        auto headers = custom_headers_;
        load_thread_ = std::jthread([this, url, headers]() {
            try {
                std::vector<WidgetValue> rows = ParseJsonRows(HttpGet(url, headers));
                std::lock_guard<std::mutex> lock(pending_mutex_);
                pending_rows_ = std::move(rows);
                has_pending_ = true;
                last_error_.clear();
            }
            catch (const std::exception& e) {
                std::cerr << "TableWidget: " << e.what() << std::endl;
                std::lock_guard<std::mutex> lock(pending_mutex_);
                last_error_ = e.what();
            }
            is_loading_ = false;
        });
    }

    void TableWidget::ApplyRows(std::vector<WidgetValue> rows) {
        live_rows_ = std::move(rows);
        live_headers_.clear();
        live_cells_.clear();

        // Колонки - ключи первой строки (как и раньше в Drawer)
        if (!live_rows_.empty()) {
            for (auto it = live_rows_[0].begin(); it != live_rows_[0].end(); ++it) {
                live_headers_.push_back(it.key());
            }
        }
        for (const auto& row : live_rows_) {
            std::vector<std::string> cells;
            cells.reserve(live_headers_.size());
            for (const auto& header : live_headers_) {
                cells.push_back(row.contains(header) ? CellToString(row[header]) : "");
            }
            live_cells_.push_back(std::move(cells));
        }

        Emit("data", live_rows_);
    }

    void TableWidget::ApplyStaticData() {
        loaded_once_ = true;

        // Строки для выхода собираем по data_field (или заголовку) настроенных колонок
        std::vector<WidgetValue> rows;
        for (const auto& cells : static_data_) {
            nlohmann::json row = nlohmann::json::object();
            for (size_t i = 0; i < columns_.size() && i < cells.size(); ++i) {
                const auto& key = columns_[i].data_field.empty() ? columns_[i].header : columns_[i].data_field;
                row[key] = cells[i];
            }
            rows.push_back(std::move(row));
        }

        live_rows_ = rows;
        live_headers_.clear();
        for (const auto& column : columns_) {
            live_headers_.push_back(column.header);
        }
        live_cells_ = static_data_;
        Emit("data", live_rows_);
    }

    void TableWidget::RenderContent(const ImVec2& screen_min, const ImVec2& screen_max) {
        ImVec2 size(screen_max.x - screen_min.x - 10, screen_max.y - screen_min.y - 10);
        ImVec2 pos(screen_min.x + 5, screen_min.y + 5);

        ImGui::SetCursorScreenPos(pos);
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(30, 30, 40, 200));

        if (ImGui::BeginChild(("##table_child_" + GetId()).c_str(), size, false,
            ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_AlwaysUseWindowPadding)) {
            if (ShowsPreview()) {
                RenderPreviewRows();
            }
            else {
                RenderLiveRows();
            }
        }
        ImGui::EndChild();

        ImGui::PopStyleColor();
        ImGui::PopStyleVar();
    }

    void TableWidget::RenderPreviewRows() {
        if (columns_.empty()) {
            ImGui::TextDisabled("No columns");
            return;
        }
        if (!ImGui::BeginTable(("##table_" + GetId()).c_str(), (int)columns_.size(), flags_, ImGui::GetContentRegionAvail())) {
            return;
        }
        for (const auto& col : columns_) {
            ImGui::TableSetupColumn(col.header.c_str(), col.sortable ? ImGuiTableColumnFlags_None : ImGuiTableColumnFlags_NoSort);
        }
        if (show_headers_) {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableHeadersRow();
        }

        // Тестовые строки
        char buf[64];
        for (int row = 0; row < 5; row++) {
            ImGui::TableNextRow();
            for (int col = 0; col < (int)columns_.size(); col++) {
                ImGui::TableSetColumnIndex(col);
                switch (col % 3) {
                case 0: snprintf(buf, sizeof(buf), "ID_%d", row + 1); break;
                case 1: snprintf(buf, sizeof(buf), "Item_%d", row + 1); break;
                default: snprintf(buf, sizeof(buf), "%.2f", (row + 1) * 10.0f); break;
                }
                ImGui::TextUnformatted(buf);
            }
        }
        ImGui::EndTable();
    }

    void TableWidget::RenderLiveRows() {
        if (live_headers_.empty()) {
            std::string error;
            {
                std::lock_guard<std::mutex> lock(pending_mutex_);
                error = last_error_;
            }
            if (!error.empty()) {
                ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", error.c_str());
            }
            else if (is_loading_) {
                ImGui::TextDisabled("Loading...");
            }
            else {
                ImGui::TextDisabled(data_source_type_ == DataSourceType::NONE ? "No data source" : "No data");
            }
            return;
        }

        if (!ImGui::BeginTable(("##table_" + GetId()).c_str(), (int)live_headers_.size(), flags_, ImGui::GetContentRegionAvail())) {
            return;
        }
        for (const auto& header : live_headers_) {
            ImGui::TableSetupColumn(header.c_str());
        }
        if (show_headers_) {
            ImGui::TableSetupScrollFreeze(0, 1);
            ImGui::TableHeadersRow();
        }

        const int rows_to_display = std::min((int)live_cells_.size(), max_display_rows_);
        ImGuiListClipper clipper;
        clipper.Begin(rows_to_display);
        while (clipper.Step()) {
            for (int row = clipper.DisplayStart; row < clipper.DisplayEnd; row++) {
                ImGui::TableNextRow();
                const auto& cells = live_cells_[row];
                for (int col = 0; col < (int)cells.size(); col++) {
                    ImGui::TableSetColumnIndex(col);
                    ImGui::TextUnformatted(cells[col].c_str());
                }
            }
        }
        ImGui::EndTable();
    }

    void TableWidget::RenderProperties() {
        ImGui::Text("==Table Settings==");
        ImGui::Separator();

        ImGui::Checkbox("Show Headers", &show_headers_);
        ImGui::Checkbox("Alternate Row Colors", &alternate_row_colors_);
        ImGui::Checkbox("Show Borders", &show_borders_);

        if (show_borders_) { flags_ |= ImGuiTableFlags_Borders; }
        else { flags_ &= ~ImGuiTableFlags_Borders; }

        if (alternate_row_colors_) { flags_ |= ImGuiTableFlags_RowBg; }
        else { flags_ &= ~ImGuiTableFlags_RowBg; }

        ImGui::Separator();
        ImGui::Text("Columns:");
        ImGui::SameLine();
        ImGui::TextDisabled("(preview, static data)");
        for (size_t i = 0; i < columns_.size(); i++) {
            ImGui::PushID((int)i);
            const float field_width = (ImGui::GetContentRegionAvail().x - 80.0f) * 0.4f;

            ImGui::SetNextItemWidth(field_width);
            ImGui::InputTextWithHint("##header", "header", &columns_[i].header);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(field_width);
            ImGui::InputTextWithHint("##field", "data field", &columns_[i].data_field);
            ImGui::SameLine();
            bool remove = ImGui::Button("Remove");
            ImGui::PopID();

            if (remove) {
                columns_.erase(columns_.begin() + i);
                i--;
            }
        }
        if (ImGui::Button("Add Column")) {
            columns_.push_back(TableColumnConfig{ "New Column", "", 100.0f, false });
        }

        ImGui::Separator();
        ImGui::Text("Data Source:");
        const char* source_types[] = { "None", "CSV URL", "JSON URL", "CSV File", "JSON File", "Static Data" };
        int current_type = (int)data_source_type_;
        if (ImGui::Combo("Type", &current_type, source_types, IM_ARRAYSIZE(source_types))) {
            data_source_type_ = (DataSourceType)current_type;
            loaded_once_ = false;
        }
        if (data_source_type_ == DataSourceType::CSV_URL || data_source_type_ == DataSourceType::CSV_FILE || data_source_type_ == DataSourceType::JSON_FILE) {
            ImGui::TextDisabled("This source type is not supported yet");
        }

        if (data_source_type_ != DataSourceType::NONE && data_source_type_ != DataSourceType::STATIC_DATA) {
            ImGui::InputText("Source", &data_source_);

            const char* triggers[] = { "None", "Timer", "Button Click", "On Load" };
            int current_trigger = (int)update_trigger_;
            if (ImGui::Combo("Update Trigger", &current_trigger, triggers, IM_ARRAYSIZE(triggers))) {
                update_trigger_ = (UpdateTrigger)current_trigger;
            }
            if (update_trigger_ == UpdateTrigger::TIMER) {
                ImGui::DragInt("Interval (seconds)", &update_interval_, 1, 5, 3600);
            }

            if (IsUrlSource() && !ShowsPreview()) {
                ImGui::BeginDisabled(is_loading_);
                if (ImGui::Button("Reload now")) {
                    StartLoading();
                }
                ImGui::EndDisabled();
            }
        }
    }

    std::vector<PortDesc> TableWidget::GetOutputPorts() const {
        return { PortDesc{ "data", "Data", "json", false } };
    }

    nlohmann::json TableWidget::ToJson() const {
        auto json = Widget::ToJson();

        nlohmann::json columns_json = nlohmann::json::array();
        for (const auto& col : columns_) {
            columns_json.push_back(col.ToJson());
        }
        json["columns"] = columns_json;

        json["data_source_type"] = (int)data_source_type_;
        json["data_source"] = data_source_;
        json["update_trigger"] = (int)update_trigger_;
        json["update_interval"] = update_interval_;

        json["flags"] = flags_;
        json["show_headers"] = show_headers_;
        json["show_borders"] = show_borders_;
        json["alternate_row_colors"] = alternate_row_colors_;
        json["max_display_rows"] = max_display_rows_;

        if (!static_data_.empty()) { json["static_data"] = static_data_; }
        if (!api_key_.empty()) { json["api_key"] = api_key_; }
        if (!custom_headers_.empty()) { json["custom_headers"] = custom_headers_; }
        return json;
    }

    void TableWidget::FromJson(const nlohmann::json& json) {
        Widget::FromJson(json);

        if (json.contains("columns") && json["columns"].is_array()) {
            columns_.clear();
            for (const auto& col_json : json["columns"]) {
                TableColumnConfig col;
                col.FromJson(col_json);
                columns_.push_back(col);
            }
        }

        data_source_type_ = (DataSourceType)json.value("data_source_type", (int)data_source_type_);
        data_source_ = json.value("data_source", data_source_);
        update_trigger_ = (UpdateTrigger)json.value("update_trigger", (int)update_trigger_);
        update_interval_ = json.value("update_interval", update_interval_);

        flags_ = json.value("flags", (int)flags_);
        show_headers_ = json.value("show_headers", show_headers_);
        show_borders_ = json.value("show_borders", show_borders_);
        alternate_row_colors_ = json.value("alternate_row_colors", alternate_row_colors_);
        max_display_rows_ = json.value("max_display_rows", max_display_rows_);

        if (json.contains("static_data") && json["static_data"].is_array()) {
            static_data_ = json["static_data"].get<std::vector<std::vector<std::string>>>();
        }
        api_key_ = json.value("api_key", api_key_);
        if (json.contains("custom_headers")) {
            custom_headers_ = json["custom_headers"].get<std::map<std::string, std::string>>();
        }
        loaded_once_ = false;
    }
}
