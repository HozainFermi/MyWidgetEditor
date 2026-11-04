#pragma once
#include <atomic>
#include <chrono>
#include <map>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include "core/Widget.h"

namespace core {

    enum class DataSourceType {
        NONE,
        CSV_URL,
        JSON_URL,
        CSV_FILE,
        JSON_FILE,
        STATIC_DATA
    };

    enum class UpdateTrigger {
        NONE,          // только вручную
        TIMER,
        BUTTON_CLICK,
        ON_LOAD
    };

    struct TableColumnConfig {
        std::string header;      // заголовок колонки
        std::string data_field;  // поле в JSON/CSV
        float width = 100.0f;
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

    // Таблица. В превью показывает настроенные колонки с тестовыми строками.
    // С живыми данными грузит JSON по URL в фоне; колонки берутся из ключей ответа.
    // Загруженные строки отправляются в выход "data" один раз после каждой загрузки.
    class TableWidget : public Widget {
    public:
        TableWidget();
        ~TableWidget() override = default;

        void Update() override;
        void RenderContent(const ImVec2& min, const ImVec2& max) override;
        void RenderProperties() override;
        void OnContextChanged() override;
        void OnPortsConnected() override;

        std::vector<PortDesc> GetOutputPorts() const override;

        nlohmann::json ToJson() const override;
        void FromJson(const nlohmann::json& json) override;

    private:
        bool IsUrlSource() const;
        void StartLoading();
        void ApplyRows(std::vector<WidgetValue> rows);
        void ApplyStaticData();
        void RenderPreviewRows();
        void RenderLiveRows();

        // === Конфигурация (сохраняется в JSON) ===
        std::vector<TableColumnConfig> columns_;
        DataSourceType data_source_type_ = DataSourceType::NONE;
        std::string data_source_;   // URL или путь к файлу
        UpdateTrigger update_trigger_ = UpdateTrigger::NONE;
        int update_interval_ = 10;  // секунды (TIMER)
        std::vector<std::vector<std::string>> static_data_;

        ImGuiTableFlags flags_;
        bool show_headers_ = true;
        bool show_borders_ = true;
        bool alternate_row_colors_ = true;
        int max_display_rows_ = 100;

        std::string api_key_;
        std::map<std::string, std::string> custom_headers_;

        // === Живые данные (только главный поток) ===
        std::vector<std::string> live_headers_;
        std::vector<std::vector<std::string>> live_cells_;
        std::vector<WidgetValue> live_rows_;
        bool loaded_once_ = false;
        std::chrono::steady_clock::time_point last_update_{};

        // === Фоновая загрузка ===
        std::mutex pending_mutex_;
        bool has_pending_ = false;               // под pending_mutex_
        std::vector<WidgetValue> pending_rows_;  // под pending_mutex_
        std::string last_error_;                 // под pending_mutex_
        std::atomic<bool> is_loading_{ false };

        // Последнее поле: уничтожается первым и дожидается потока, пока остальные поля ещё живы
        std::jthread load_thread_;
    };
}
