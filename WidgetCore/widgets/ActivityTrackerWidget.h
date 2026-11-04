#pragma once
#include <chrono>
#include <map>
#include <string>
#include "core/Widget.h"

namespace core {

    // Время в программах за сегодня. Раз в секунду смотрит, какое окно активно (Windows),
    // и копит секунды по имени exe. Пока пользователь не трогает мышь/клавиатуру дольше
    // idle-порога, время не считается.
    // Считает только Drawer; редактор с живыми данными лишь читает тот же файл.
    // Выход "usage": [{app, minutes, seconds, rank}] раз в минуту.
    class ActivityTrackerWidget : public Widget {
    public:
        ActivityTrackerWidget();
        ~ActivityTrackerWidget() override;

        void Update() override;
        void Render(ImDrawList* draw_list, const ImVec2& min, const ImVec2& max) override;
        void RenderProperties() override;
        void OnContextChanged() override;
        void OnPortsConnected() override;
        float GetDesiredFps() const override { return 1.0f; }  // данные меняются раз в секунду

        std::vector<PortDesc> GetOutputPorts() const override;

        nlohmann::json ToJson() const override;
        void FromJson(const nlohmann::json& json) override;

    private:
        bool IsTracking() const;
        const std::map<std::string, double>& Shown() const;
        std::string DataFilePath() const;
        void LoadFile();
        void SaveFile();
        void EmitUsage();

        // === Настройки (сохраняются) ===
        int top_count_ = 6;
        int idle_threshold_sec_ = 120;
        std::string data_file_;  // пусто = %LOCALAPPDATA%/MyWidgetEditor/activity.json

        // === Данные ===
        std::string today_;                  // YYYY-MM-DD
        std::map<std::string, double> usage_;  // exe -> секунды за сегодня
        std::map<std::string, double> preview_;
        nlohmann::json other_days_ = nlohmann::json::object();
        std::string current_app_;
        bool is_idle_ = false;

        std::chrono::steady_clock::time_point last_tick_{};
        std::chrono::steady_clock::time_point last_save_{};
        std::chrono::steady_clock::time_point last_emit_{};
        std::chrono::steady_clock::time_point last_read_{};
        bool loaded_ = false;
        // Копия !Context().is_editor: в деструкторе контекст менеджера уже может быть уничтожен
        bool tracking_ = true;
    };
}
