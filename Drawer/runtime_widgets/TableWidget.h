#pragma once
#include "Widget.h"
#include <string>
#include "../managers/RuntimeWidgetFactory.h"

namespace rn {

    enum class CellContentType {
        TEXT,
        BUTTON,
        INPUT_TEXT,
        CHECKBOX,
        PROGRESS_BAR
    };

    enum class DataSourceType {
        NONE,
        CSV_URL,
        JSON_URL,
        CSV_FILE,
        JSON_FILE,
        STATIC_DATA
    };

    enum class UpdateTrigger {
        NONE,          // Только ручное
        TIMER,
        BUTTON_CLICK,
        ON_LOAD
    };

    struct TableColumnConfig {
        std::string header;
        float width = 100.0f;
        std::string data_field;  // Поле в JSON/CSV (например: "name", "price")
        bool sortable = false;

        nlohmann::json ToJson() const {
            return {
                {"header", header},
                {"width", width},
                {"data_field", data_field},
                {"sortable", sortable}
            };
        }

        void FromJson(const nlohmann::json& json) {
            header = json.value("header", "");
            width = json.value("width", 100.0f);
            data_field = json.value("data_field", "");
            sortable = json.value("sortable", false);
        }
    };

    class TableWidget : public Widget {
    private:
        // === КОНФИГУРАЦИЯ ===
        std::vector<TableColumnConfig> columns_;
        DataSourceType data_source_type_ = DataSourceType::NONE;
        std::string data_source_;  // URL или путь к файлу
        UpdateTrigger update_trigger_ = UpdateTrigger::NONE;
        float update_interval_ = 5.0f;  // в секундах (если TIMER)

        // Статические данные (если источник STATIC_DATA)
        std::vector<std::vector<std::string>> static_data_;

        // Настройки таблицы
        ImGuiTableFlags flags_;
        bool show_headers_ = true;
        bool show_borders_ = true;
        bool alternate_row_colors_ = true;
        int max_display_rows_ = 100;  // Ограничение для производительности
        bool column_selected[3] = {};

        // Стиль (сериализуется)
        ImU32 header_bg_color_ = IM_COL32(60, 60, 80, 255);
        ImU32 row_bg_color_ = IM_COL32(40, 40, 50, 200);
        ImU32 alternate_row_bg_color_ = IM_COL32(50, 50, 60, 200);

        // HTTP настройки (если URL)
        std::string api_key_;
        std::map<std::string, std::string> custom_headers_;

    public:
        TableWidget();
        TableWidget(const std::string& name, const ImVec2& pos);
        bool UpdateInteraction(int widget_id) override;
        void Render(ImDrawList* draw_list) override;
        void RenderContent(ImVec2& screen_min, ImVec2& screen_max) override;

        void RenderProperties();

        // === КОНФИГУРАЦИОННЫЕ МЕТОДЫ ===
        void AddColumn(const std::string& header, float width = 100.0f,
            const std::string& data_field = "");
        void RemoveColumn(int index);

        void SetDataSource(const std::string& source, DataSourceType type);
        void SetUpdateTrigger(UpdateTrigger trigger, float interval = 5.0f);
        void SetStaticData(const std::vector<std::vector<std::string>>& data);

        void SetApiKey(const std::string& api_key) { api_key_ = api_key; }
        void AddCustomHeader(const std::string& key, const std::string& value) {
            custom_headers_[key] = value;
        }

        // === ГЕТТЕРЫ (для UI редактора) ===
        const std::vector<TableColumnConfig>& GetColumns() const { return columns_; }
        DataSourceType GetDataSourceType() const { return data_source_type_; }
        const std::string& GetDataSource() const { return data_source_; }
        UpdateTrigger GetUpdateTrigger() const { return update_trigger_; }
        float GetUpdateInterval() const { return update_interval_; }

        // === СЕРИАЛИЗАЦИЯ ===
        nlohmann::json ToJson() const override;
        void FromJson(const nlohmann::json& json) override;

   
    };
}