#include "TableWidget.h"
#include <imgui_internal.h>

namespace rn {

    REGISTER_WIDGET(TableWidget);

    TableWidget::TableWidget() :
        Widget("Unnamed", WidgetType::TABLE, ImVec2(20, 20), ImVec2(400, 300)) {  // ˜˜˜˜˜˜˜ ˜˜˜˜˜˜ ˜˜ ˜˜˜˜˜˜˜˜˜
        flags_ = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

        // ˜˜˜˜˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜ ˜˜˜ ˜˜˜˜˜˜
        //columns_.push_back(std::pair(TableColumnConfig{ "ID", "id",50.0f, true }, std::vector < std::string>{}));
        //columns_.push_back(std::pair(TableColumnConfig{ "Name", "name", 150.0f,true }, std::vector < std::string>{}));
        //columns_.push_back(std::pair(TableColumnConfig{ "Value", "value",100.0f, false }, std::vector < std::string>{}) );

        SetWidgetClass("TableWidget");
    }

    TableWidget::TableWidget(const std::string& name, const ImVec2& pos) :
        Widget(name, WidgetType::TABLE, pos, ImVec2(400, 300)) {  // ˜˜˜˜˜˜˜ ˜˜˜˜˜˜ ˜˜ ˜˜˜˜˜˜˜˜˜
        flags_ = ImGuiTableFlags_Borders | ImGuiTableFlags_RowBg;

        // ˜˜˜˜˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜ ˜˜˜ ˜˜˜˜˜˜
       //columns_.push_back(std::pair(TableColumnConfig{ "ID", "id", 50.0f,true }, std::vector < std::string>{}) );
       //columns_.push_back(std::pair(TableColumnConfig{ "Name", "name", 150.0f,true }, std::vector < std::string>{}));
       //columns_.push_back(std::pair(TableColumnConfig{ "Value", "value",100.0f, false }, std::vector < std::string>{}) );

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
        // Îãğàíè÷èâàåì îáëàñòü òàáëèöû
        ImGui::SetNextWindowPos(pos);
        ImGui::SetNextWindowSize(size);

        // Èñïîëüçóåì îêíî áåç çàãîëîâêà äëÿ îãğàíè÷åíèÿ îáëàñòè
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0, 0));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, IM_COL32(30, 30, 40, 200));
        if (update_trigger_!=UpdateTrigger::NONE) {
            if(!is_loading_){
               UpdateTableData();
            }
        }

        if (ImGui::BeginChild(("##table_child_" + GetId()).c_str(),
            size,
            false, // No border
            ImGuiWindowFlags_NoMove |
            ImGuiWindowFlags_NoScrollbar |
            ImGuiWindowFlags_AlwaysUseWindowPadding)) {

            // Ïîëó÷àåì äîñòóïíóş îáëàñòü âíóòğè îêíà
            ImVec2 content_size = ImGui::GetContentRegionAvail();

            
            if (ImGui::BeginTable(("##table_" + GetId()).c_str(),
                (int)columns_.size(),
                flags_,
                content_size)) {

                // Óñòàíàâëèâàåì çàãîëîâêè êîëîíîê
                for (const auto& col : columns_) {
                    ImGui::TableSetupColumn(col.first.header.c_str(),
                        col.first.sortable ? ImGuiTableColumnFlags_None :
                        ImGuiTableColumnFlags_NoSort,
                        0.0f);//col.first.width
                }

                if (show_headers_) {
                    ImGui::TableSetupScrollFreeze(0, 1); // Çàãîëîâîê ôèêñèğîâàí ïğè ñêğîëëå
                    ImGui::TableHeadersRow();
                }

                static char buf[256];               
                // Îïğåäåëÿåì ğåàëüíîå êîëè÷åñòâî ñòğîê äëÿ îòîáğàæåíèÿ
                int rows_to_display = 0;
                if (!columns_.empty() && !columns_[0].second.empty()) {
                    rows_to_display = std::min((int)columns_[0].second.size(), max_display_rows_);
                }

                for (int row = 0; row < rows_to_display; row++) {
                    ImGui::TableNextRow();

                    for (int col = 0; col < (int)columns_.size(); col++) {
                        ImGui::TableSetColumnIndex(col);

                        if (row < (int)columns_[col].second.size()) {
                            sprintf_s(buf, columns_[col].second[row].c_str());
                        }
                        else {
                            sprintf_s(buf, "");
                        }

                        ImGui::PushID(row * 10 + col);
                        ImGui::Selectable(buf, column_selected[col]);
                        ImGui::PopID();
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
        columns_.push_back(std::pair(TableColumnConfig{ header, data_field,width, false }, std::vector < std::string>{}) );
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

    

    httplib::Result TableWidget::LoadTableData()
    {

        for (auto& element : columns_) {
            element.second.clear();
        }

        if (data_source_type_ != DataSourceType::NONE &&
            data_source_type_ != DataSourceType::STATIC_DATA) {

            //URL
            auto hostNquery = ParseUrl();
            
            httplib::Client cli(hostNquery.first.c_str());
            
            if (auto res = cli.Get(hostNquery.second.c_str())) {
                if (data_source_type_ == DataSourceType::JSON_URL) {
                    std::cout << "RESPONSE: " << res->body.substr(0, 15) << std::endl;
                    FromResponseJsonToColumns(res->body);
                }
                return res;
            }
            else {
                throw std::runtime_error("Can't get data. Error: " +
                    std::string(httplib::to_string(res.error())));
            }
        }
        else {
            return httplib::Result{};
        }
    }



    bool TableWidget::UpdateTableData()
    {        
        switch (update_trigger_) {


        case UpdateTrigger::TIMER:
            now_ = std::chrono::steady_clock::now();
            auto time_since_last_update = now_ - last_update_;

            if (time_since_last_update >= update_interval_millisec_) {
                if (!is_loading_) {
                    is_loading_ = true;
                    load_thread_ = std::jthread([this]() {
                        try {
                            LoadTableData();
                        } catch (...) {
                        }
                        is_loading_ = false;
                    });
                }
                last_update_ = std::chrono::steady_clock::now();
                return true;
            }            
            break;


        case UpdateTrigger::BUTTON_CLICK:
            break;


        case UpdateTrigger::ON_LOAD:
            if (last_update_.time_since_epoch().count() == 0 && !is_loading_) {
                is_loading_ = true;
                load_thread_ = std::jthread([this]() {
                    try {
                        LoadTableData();
                    } catch (...) {
                    }
                    is_loading_ = false;
                });
                last_update_ = std::chrono::steady_clock::now();
            }
            break;
       }


        return false;

        
    }

   

    void TableWidget::FromResponseJsonToColumns(const std::string& body)
    {
        try {          
            nlohmann::json json = nlohmann::json::parse(body);

            if (json.is_array()) {
                max_display_rows_ = (int)json.size();
                rows_.clear();
                if (columns_.empty() && !json.empty() && json[0].is_object()) {
                    for (auto& [key, value] : json[0].items()) {
                        columns_.push_back(std::pair{ TableColumnConfig{ key, key,50.0f, false }, std::vector<std::string>{} });
                    }
                }

                for (auto& line : json) {
                    if (line.is_object()) {
                        if (line.size() == columns_.size()) {
                            int i = 0;
                            for (auto& [key, value] : line.items()) {                                
                                columns_[i].second.push_back(value.dump());
                                i++;
                            }
                            rows_.push_back(line);
                        }
                        else {
                            std::cout << "Line has " << line.size() << " fields, but table has " << columns_.size() << " columns" << std::endl;
                            throw std::invalid_argument("Response body object is not the same size as the table");
                        }
                    }
                    else {
                        throw std::invalid_argument("Element in array is not an object");
                    }
                }

                // Îòïğàâëÿåì äàííûå äàëüøå
                Emit("data", rows_);
            }
            else {
                throw std::invalid_argument("Response body is not an array");
            }            
        }
        catch (const nlohmann::json::parse_error& e) {
            std::cout << "JSON parse error: " << e.what() << std::endl;
            throw std::invalid_argument("Invalid JSON format");
        }
    }

    void TableWidget::FromJson(const nlohmann::json& json) {
        Widget::FromJson(json);

        // ˜˜˜˜˜˜˜
        if (json.contains("columns") && json["columns"].is_array()) {
            columns_.clear();
            for (const auto& col_json : json["columns"]) {
                TableColumnConfig col;
                col.FromJson(col_json);
                columns_.push_back(std::pair{ col,std::vector<std::string>{} });
            }
        }

        // ˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜
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
            update_interval_ = json["update_interval"].get<int>();
            update_interval_millisec_= std::chrono::milliseconds{ update_interval_ * 1000 };
        }

        // ˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜
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

        // ˜˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜
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

        // HTTP ˜˜˜˜˜˜˜˜˜
        if (json.contains("api_key")) {
            api_key_ = json["api_key"].get<std::string>();
        }
        if (json.contains("custom_headers")) {
            custom_headers_ = json["custom_headers"].get<std::map<std::string, std::string>>();
        }
    }



    //==Helper functions==

    void TableWidget::GenerateData() {
        // ˜˜˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜˜˜ ˜˜˜˜˜˜ ˜˜˜ ˜˜˜˜˜˜
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

    std::pair<std::string, std::string> TableWidget::ParseUrl() {
        size_t protocolEnd = data_source_.find("://");
        if (protocolEnd == std::string::npos) {
            throw std::invalid_argument("Invalid URL: " + data_source_);
        }

        size_t domainStart = protocolEnd + 3;
        size_t pathStart = data_source_.find('/', domainStart);

        std::string scheme_host_port;
        std::string path_query;

        if (pathStart == std::string::npos) {
            scheme_host_port = data_source_;
            path_query = "/";
        }
        else {
            scheme_host_port = data_source_.substr(0, pathStart);
            path_query = data_source_.substr(pathStart);
        }

        return std::pair<std::string, std::string>{scheme_host_port, path_query};
    }
}