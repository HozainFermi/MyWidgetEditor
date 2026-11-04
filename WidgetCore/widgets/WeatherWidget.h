#pragma once
#include <atomic>
#include <chrono>
#include <mutex>
#include <string>
#include <thread>
#include <vector>
#include "core/Widget.h"

namespace core {

    struct DailyForecast {
        std::string date;  // YYYY-MM-DD
        int code = 0;      // код погоды WMO
        float temp_max = 0.0f;
        float temp_min = 0.0f;
    };

    struct WeatherData {
        bool valid = false;
        float temperature = 0.0f;
        float feels_like = 0.0f;
        int humidity = 0;
        float wind = 0.0f;
        int code = 0;
        bool is_day = true;
        std::vector<DailyForecast> daily;
    };

    // Погода из Open-Meteo (бесплатно, без ключа). Город ищется геокодером Open-Meteo.
    // Всё рисуется кодом: градиент под погоду и время суток, анимированные иконки, прогноз на несколько дней.
    // Выход "weather": [{temperature, feels_like, humidity, wind, code, description}] после каждой загрузки.
    class WeatherWidget : public Widget {
    public:
        WeatherWidget();

        void Update() override;
        void Render(ImDrawList* draw_list, const ImVec2& min, const ImVec2& max) override;
        void RenderProperties() override;
        void OnContextChanged() override;
        void OnPortsConnected() override;

        std::vector<PortDesc> GetOutputPorts() const override;

        nlohmann::json ToJson() const override;
        void FromJson(const nlohmann::json& json) override;

    private:
        void StartLoading(bool find_city);
        const WeatherData& Shown() const;
        void EmitWeather();

        // === Настройки (сохраняются) ===
        std::string city_ = "Москва";
        std::string location_name_ = "Москва";
        double latitude_ = 55.7558;
        double longitude_ = 37.6173;
        bool fahrenheit_ = false;
        int update_interval_min_ = 30;
        int forecast_days_ = 3;

        // === Данные (главный поток) ===
        WeatherData data_;
        WeatherData preview_;
        bool loaded_once_ = false;
        std::chrono::steady_clock::time_point last_update_{};

        // === Фоновая загрузка ===
        std::mutex pending_mutex_;
        bool has_pending_ = false;          // под pending_mutex_
        WeatherData pending_data_;          // под pending_mutex_
        bool has_pending_location_ = false; // под pending_mutex_
        std::string pending_location_;      // под pending_mutex_
        double pending_latitude_ = 0.0;     // под pending_mutex_
        double pending_longitude_ = 0.0;    // под pending_mutex_
        std::string last_error_;            // под pending_mutex_
        std::atomic<bool> is_loading_{ false };

        // Последнее поле: уничтожается первым и дожидается потока
        std::jthread load_thread_;
    };
}
