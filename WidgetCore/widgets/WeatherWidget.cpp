#include "WeatherWidget.h"
#include <algorithm>
#include <cmath>
#include <iostream>
#include <sstream>
#include <imgui_internal.h>
#include <imgui_stdlib.h>
#include "core/Http.h"

namespace core {

    namespace {
        enum class Sky { Clear, PartlyCloudy, Cloudy, Fog, Rain, Snow, Thunder };

        struct CodeInfo {
            const char* description;
            Sky sky;
        };

        // Коды погоды WMO, которые отдаёт Open-Meteo
        CodeInfo DescribeCode(int code) {
            switch (code) {
            case 0: return { "Ясно", Sky::Clear };
            case 1: return { "Преимущественно ясно", Sky::PartlyCloudy };
            case 2: return { "Переменная облачность", Sky::PartlyCloudy };
            case 3: return { "Пасмурно", Sky::Cloudy };
            case 45: case 48: return { "Туман", Sky::Fog };
            case 51: case 53: case 55: return { "Морось", Sky::Rain };
            case 56: case 57: return { "Ледяная морось", Sky::Rain };
            case 61: return { "Небольшой дождь", Sky::Rain };
            case 63: return { "Дождь", Sky::Rain };
            case 65: return { "Сильный дождь", Sky::Rain };
            case 66: case 67: return { "Ледяной дождь", Sky::Rain };
            case 71: return { "Небольшой снег", Sky::Snow };
            case 73: return { "Снег", Sky::Snow };
            case 75: return { "Сильный снег", Sky::Snow };
            case 77: return { "Снежные зёрна", Sky::Snow };
            case 80: case 81: return { "Ливень", Sky::Rain };
            case 82: return { "Сильный ливень", Sky::Rain };
            case 85: case 86: return { "Снегопад", Sky::Snow };
            case 95: return { "Гроза", Sky::Thunder };
            case 96: case 99: return { "Гроза с градом", Sky::Thunder };
            }
            return { "Неизвестно", Sky::Cloudy };
        }

        struct Gradient { ImU32 top, bottom; };

        Gradient SkyGradient(Sky sky, bool is_day) {
            if (!is_day) {
                return { IM_COL32(14, 18, 42, 235), IM_COL32(40, 48, 88, 235) };
            }
            switch (sky) {
            case Sky::Clear:        return { IM_COL32(45, 110, 215, 235), IM_COL32(90, 160, 235, 235) };
            case Sky::PartlyCloudy: return { IM_COL32(70, 125, 200, 235), IM_COL32(140, 180, 225, 235) };
            case Sky::Cloudy:       return { IM_COL32(95, 108, 128, 235), IM_COL32(150, 162, 178, 235) };
            case Sky::Fog:          return { IM_COL32(92, 98, 108, 235), IM_COL32(135, 140, 148, 235) };
            case Sky::Rain:         return { IM_COL32(55, 70, 98, 235), IM_COL32(95, 115, 140, 235) };
            case Sky::Snow:         return { IM_COL32(84, 106, 146, 235), IM_COL32(132, 154, 190, 235) };
            case Sky::Thunder:      return { IM_COL32(42, 36, 66, 235), IM_COL32(85, 75, 110, 235) };
            }
            return { IM_COL32(60, 60, 60, 235), IM_COL32(90, 90, 90, 235) };
        }

        void DrawCloud(ImDrawList* dl, ImVec2 c, float r, ImU32 col) {
            dl->AddCircleFilled(ImVec2(c.x - r * 0.45f, c.y + r * 0.1f), r * 0.42f, col, 24);
            dl->AddCircleFilled(ImVec2(c.x + r * 0.05f, c.y - r * 0.15f), r * 0.55f, col, 24);
            dl->AddCircleFilled(ImVec2(c.x + r * 0.55f, c.y + r * 0.12f), r * 0.38f, col, 24);
            dl->AddRectFilled(ImVec2(c.x - r * 0.45f, c.y + r * 0.1f), ImVec2(c.x + r * 0.55f, c.y + r * 0.5f), col, r * 0.2f);
        }

        void DrawSun(ImDrawList* dl, ImVec2 c, float r, float time) {
            const ImU32 col = IM_COL32(255, 205, 60, 255);
            const float spin = time * 0.4f;
            for (int i = 0; i < 8; ++i) {
                float a = spin + i * IM_PI / 4.0f;
                dl->AddLine(ImVec2(c.x + cosf(a) * r * 0.72f, c.y + sinf(a) * r * 0.72f),
                    ImVec2(c.x + cosf(a) * r, c.y + sinf(a) * r), col, ImMax(1.5f, r * 0.09f));
            }
            dl->AddCircleFilled(c, r * 0.55f, col, 32);
        }

        void DrawMoon(ImDrawList* dl, ImVec2 c, float r, ImU32 sky_color) {
            dl->AddCircleFilled(c, r * 0.6f, IM_COL32(235, 235, 210, 255), 32);
            dl->AddCircleFilled(ImVec2(c.x + r * 0.28f, c.y - r * 0.18f), r * 0.52f, sky_color, 32);
        }

        // Иконка погоды с простой анимацией (time - секунды)
        void DrawWeatherIcon(ImDrawList* dl, ImVec2 c, float r, Sky sky, bool is_day, float time, ImU32 sky_color) {
            const ImU32 cloud = IM_COL32(240, 244, 250, 255);
            const ImU32 dark_cloud = IM_COL32(150, 155, 170, 255);

            switch (sky) {
            case Sky::Clear:
                if (is_day) DrawSun(dl, c, r, time); else DrawMoon(dl, c, r, sky_color);
                break;
            case Sky::PartlyCloudy:
                if (is_day) DrawSun(dl, ImVec2(c.x - r * 0.3f, c.y - r * 0.3f), r * 0.75f, time);
                else DrawMoon(dl, ImVec2(c.x - r * 0.3f, c.y - r * 0.3f), r * 0.75f, sky_color);
                DrawCloud(dl, ImVec2(c.x + r * 0.1f, c.y + r * 0.15f), r * 0.85f, cloud);
                break;
            case Sky::Cloudy:
                DrawCloud(dl, ImVec2(c.x - r * 0.25f, c.y - r * 0.2f), r * 0.7f, dark_cloud);
                DrawCloud(dl, ImVec2(c.x + r * 0.1f, c.y + r * 0.05f), r * 0.9f, cloud);
                break;
            case Sky::Fog:
                for (int i = 0; i < 4; ++i) {
                    float y = c.y - r * 0.5f + i * r * 0.33f;
                    float shift = sinf(time * 0.8f + i) * r * 0.1f;
                    dl->AddLine(ImVec2(c.x - r * 0.8f + shift, y), ImVec2(c.x + r * 0.8f + shift, y), cloud, ImMax(2.0f, r * 0.12f));
                }
                break;
            case Sky::Rain:
            case Sky::Thunder: {
                DrawCloud(dl, ImVec2(c.x, c.y - r * 0.25f), r * 0.9f, sky == Sky::Thunder ? dark_cloud : cloud);
                // Капли падают и появляются заново
                const ImU32 drop = IM_COL32(120, 190, 255, 255);
                for (int i = 0; i < 4; ++i) {
                    float phase = fmodf(time * 1.6f + i * 0.37f, 1.0f);
                    float x = c.x - r * 0.45f + i * r * 0.3f;
                    float y = c.y + r * 0.3f + phase * r * 0.6f;
                    dl->AddLine(ImVec2(x, y), ImVec2(x - r * 0.08f, y + r * 0.2f), drop, ImMax(1.5f, r * 0.07f));
                }
                if (sky == Sky::Thunder && fmodf(time, 2.5f) < 0.25f) {
                    // Короткая вспышка молнии раз в пару секунд
                    ImVec2 bolt[] = {
                        ImVec2(c.x + r * 0.05f, c.y + r * 0.1f), ImVec2(c.x - r * 0.15f, c.y + r * 0.45f),
                        ImVec2(c.x + r * 0.05f, c.y + r * 0.45f), ImVec2(c.x - r * 0.1f, c.y + r * 0.85f) };
                    dl->AddPolyline(bolt, 4, IM_COL32(255, 230, 80, 255), 0, ImMax(2.0f, r * 0.1f));
                }
                break;
            }
            case Sky::Snow: {
                DrawCloud(dl, ImVec2(c.x, c.y - r * 0.25f), r * 0.9f, cloud);
                for (int i = 0; i < 5; ++i) {
                    float phase = fmodf(time * 0.5f + i * 0.23f, 1.0f);
                    float x = c.x - r * 0.5f + i * r * 0.25f + sinf(time * 2.0f + i) * r * 0.05f;
                    float y = c.y + r * 0.3f + phase * r * 0.6f;
                    dl->AddCircleFilled(ImVec2(x, y), ImMax(1.5f, r * 0.07f), IM_COL32(255, 255, 255, 255), 8);
                }
                break;
            }
            }
        }

        std::string FormatTemp(float value) {
            return std::to_string((int)std::lround(value)) + "°";
        }

        const char* WeekdayName(const std::string& date) {
            static const char* kNames[] = { "Вс", "Пн", "Вт", "Ср", "Чт", "Пт", "Сб" };
            int y = 0, m = 0, d = 0;
            if (sscanf_s(date.c_str(), "%d-%d-%d", &y, &m, &d) != 3) {
                return "?";
            }
            std::chrono::year_month_day ymd{ std::chrono::year(y), std::chrono::month(m), std::chrono::day(d) };
            return kNames[std::chrono::weekday(std::chrono::sys_days(ymd)).c_encoding()];
        }

        float TextWidth(ImFont* font, float size, const std::string& text) {
            return font->CalcTextSizeA(size, FLT_MAX, 0.0f, text.c_str()).x;
        }

        WeatherData ParseForecast(const std::string& body) {
            nlohmann::json json = nlohmann::json::parse(body);
            const auto& current = json.at("current");

            WeatherData data;
            data.temperature = current.value("temperature_2m", 0.0f);
            data.feels_like = current.value("apparent_temperature", data.temperature);
            data.humidity = (int)current.value("relative_humidity_2m", 0.0f);
            data.wind = current.value("wind_speed_10m", 0.0f);
            data.code = current.value("weather_code", 0);
            data.is_day = current.value("is_day", 1) != 0;

            if (json.contains("daily")) {
                const auto& daily = json["daily"];
                const auto& dates = daily.at("time");
                for (size_t i = 0; i < dates.size(); ++i) {
                    DailyForecast day;
                    day.date = dates[i].get<std::string>();
                    day.code = daily.at("weather_code")[i].get<int>();
                    day.temp_max = daily.at("temperature_2m_max")[i].get<float>();
                    day.temp_min = daily.at("temperature_2m_min")[i].get<float>();
                    data.daily.push_back(day);
                }
            }
            data.valid = true;
            return data;
        }
    }

    WeatherWidget::WeatherWidget()
        : Widget(WidgetType::CUSTOM, ImVec2(320, 220)) {
        // Данные для превью в редакторе
        preview_.valid = true;
        preview_.temperature = -3.0f;
        preview_.feels_like = -8.0f;
        preview_.humidity = 86;
        preview_.wind = 5.0f;
        preview_.code = 73;
        preview_.is_day = true;
        preview_.daily = {
            { "2026-11-04", 73, -2.0f, -6.0f },
            { "2026-11-05", 3, 0.0f, -4.0f },
            { "2026-11-06", 61, 3.0f, -1.0f },
            { "2026-11-07", 0, 1.0f, -5.0f },
            { "2026-11-08", 95, 5.0f, 1.0f },
        };
    }

    const WeatherData& WeatherWidget::Shown() const {
        return ShowsPreview() ? preview_ : data_;
    }

    void WeatherWidget::OnContextChanged() {
        loaded_once_ = false;
    }

    void WeatherWidget::OnPortsConnected() {
        if (!ShowsPreview() && data_.valid) {
            EmitWeather();
        }
    }

    std::vector<PortDesc> WeatherWidget::GetOutputPorts() const {
        return { PortDesc{ "weather", "Weather", "json", false } };
    }

    void WeatherWidget::EmitWeather() {
        Emit("weather", { {
            {"temperature", data_.temperature},
            {"feels_like", data_.feels_like},
            {"humidity", data_.humidity},
            {"wind", data_.wind},
            {"code", data_.code},
            {"description", DescribeCode(data_.code).description}
        } });
    }

    void WeatherWidget::Update() {
        if (ShowsPreview()) {
            return;
        }

        bool got_data = false;
        {
            std::lock_guard<std::mutex> lock(pending_mutex_);
            if (has_pending_location_) {
                location_name_ = pending_location_;
                latitude_ = pending_latitude_;
                longitude_ = pending_longitude_;
                has_pending_location_ = false;
            }
            if (has_pending_) {
                data_ = std::move(pending_data_);
                has_pending_ = false;
                got_data = true;
            }
        }
        if (got_data) {
            EmitWeather();
        }

        if (is_loading_) {
            return;
        }
        const auto now = std::chrono::steady_clock::now();
        if (!loaded_once_ || now - last_update_ >= std::chrono::minutes(update_interval_min_)) {
            StartLoading(false);
        }
    }

    void WeatherWidget::StartLoading(bool find_city) {
        loaded_once_ = true;
        last_update_ = std::chrono::steady_clock::now();
        is_loading_ = true;
        if (load_thread_.joinable()) {
            load_thread_.join();
        }

        // Поток получает копии настроек и пишет только в pending-поля
        std::string city = city_;
        double latitude = latitude_;
        double longitude = longitude_;
        bool fahrenheit = fahrenheit_;
        int days = std::clamp(forecast_days_ + 1, 1, 16);  // +1: сегодняшний день

        load_thread_ = std::jthread([this, find_city, city, latitude, longitude, fahrenheit, days]() mutable {
            try {
                if (find_city) {
                    auto geo = nlohmann::json::parse(HttpGet(
                        "https://geocoding-api.open-meteo.com/v1/search?count=1&language=ru&format=json&name=" + UrlEncode(city)));
                    if (!geo.contains("results") || geo["results"].empty()) {
                        throw std::runtime_error("Город не найден: " + city);
                    }
                    const auto& place = geo["results"][0];
                    latitude = place.at("latitude").get<double>();
                    longitude = place.at("longitude").get<double>();
                    std::string name = place.value("name", city);
                    if (place.contains("country")) {
                        name += ", " + place["country"].get<std::string>();
                    }
                    std::lock_guard<std::mutex> lock(pending_mutex_);
                    pending_location_ = name;
                    pending_latitude_ = latitude;
                    pending_longitude_ = longitude;
                    has_pending_location_ = true;
                }

                std::ostringstream url;
                url.precision(4);
                url << std::fixed << "https://api.open-meteo.com/v1/forecast?latitude=" << latitude << "&longitude=" << longitude
                    << "&current=temperature_2m,apparent_temperature,relative_humidity_2m,weather_code,wind_speed_10m,is_day"
                    << "&daily=weather_code,temperature_2m_max,temperature_2m_min"
                    << "&wind_speed_unit=ms&timezone=auto&forecast_days=" << days;
                if (fahrenheit) {
                    url << "&temperature_unit=fahrenheit";
                }

                WeatherData data = ParseForecast(HttpGet(url.str()));
                std::lock_guard<std::mutex> lock(pending_mutex_);
                pending_data_ = std::move(data);
                has_pending_ = true;
                last_error_.clear();
            }
            catch (const std::exception& e) {
                std::cerr << "WeatherWidget: " << e.what() << std::endl;
                std::lock_guard<std::mutex> lock(pending_mutex_);
                last_error_ = e.what();
            }
            is_loading_ = false;
        });
    }

    void WeatherWidget::Render(ImDrawList* dl, const ImVec2& min, const ImVec2& max) {
        const WeatherData& w = Shown();
        const float time = (float)ImGui::GetTime();
        const float width = max.x - min.x;
        const float height = max.y - min.y;
        // Всё масштабируется от высоты: дефолтный размер 320x220
        const float k = std::clamp(height / 220.0f, 0.6f, 3.0f);
        const float pad = 12.0f * k;
        ImFont* font = ImGui::GetFont();

        dl->PushClipRect(min, max, true);

        CodeInfo info = DescribeCode(w.code);
        Gradient g = SkyGradient(info.sky, w.is_day);
        dl->AddRectFilledMultiColor(min, max, g.top, g.top, g.bottom, g.bottom);

        const ImU32 white = IM_COL32(255, 255, 255, 255);
        const ImU32 soft = IM_COL32(255, 255, 255, 190);

        std::string error;
        {
            std::lock_guard<std::mutex> lock(pending_mutex_);
            error = last_error_;
        }

        // Город и описание
        float y = min.y + pad;
        dl->AddText(font, 16.0f * k, ImVec2(min.x + pad, y), white, location_name_.c_str());
        y += 18.0f * k;

        if (!w.valid) {
            const char* status = !error.empty() ? error.c_str() : (is_loading_ ? "Загрузка..." : "Нет данных");
            dl->AddText(font, 13.0f * k, ImVec2(min.x + pad, y + 4.0f * k), soft, status, nullptr, width - pad * 2);
            dl->PopClipRect();
            return;
        }

        dl->AddText(font, 13.0f * k, ImVec2(min.x + pad, y), soft, info.description);
        y += 16.0f * k;

        // Крупная температура и иконка справа
        std::string temp = FormatTemp(w.temperature);
        dl->AddText(font, 52.0f * k, ImVec2(min.x + pad, y), white, temp.c_str());

        const float icon_r = std::min(38.0f * k, width * 0.18f);
        DrawWeatherIcon(dl, ImVec2(max.x - pad - icon_r, min.y + pad + icon_r + 4.0f * k), icon_r, info.sky, w.is_day, time, g.top);

        y += 56.0f * k;
        std::string details = "Ощущается " + FormatTemp(w.feels_like) +
            "   Влажность " + std::to_string(w.humidity) + "%" +
            "   Ветер " + std::to_string((int)std::lround(w.wind)) + " м/с";
        dl->AddText(font, 12.0f * k, ImVec2(min.x + pad, y), soft, details.c_str(), nullptr, width - pad * 2);

        // Прогноз: дни после сегодняшнего
        const int days = std::min(forecast_days_, (int)w.daily.size() - 1);
        if (days > 0) {
            const float row_top = max.y - pad - 54.0f * k;
            if (row_top > y + 14.0f * k) {
                dl->AddLine(ImVec2(min.x + pad, row_top - 6.0f * k), ImVec2(max.x - pad, row_top - 6.0f * k), IM_COL32(255, 255, 255, 60));
                const float cell = (width - pad * 2) / days;
                for (int i = 0; i < days; ++i) {
                    const DailyForecast& day = w.daily[i + 1];
                    const float cx = min.x + pad + cell * (i + 0.5f);

                    std::string name = WeekdayName(day.date);
                    dl->AddText(font, 12.0f * k, ImVec2(cx - TextWidth(font, 12.0f * k, name) * 0.5f, row_top), soft, name.c_str());

                    DrawWeatherIcon(dl, ImVec2(cx, row_top + 26.0f * k), 11.0f * k, DescribeCode(day.code).sky, true, time, g.bottom);

                    std::string range = FormatTemp(day.temp_max) + " " + FormatTemp(day.temp_min);
                    dl->AddText(font, 12.0f * k, ImVec2(cx - TextWidth(font, 12.0f * k, range) * 0.5f, row_top + 42.0f * k), white, range.c_str());
                }
            }
        }

        dl->PopClipRect();
    }

    void WeatherWidget::RenderProperties() {
        ImGui::Text("==Weather==");
        ImGui::Separator();

        ImGui::SetNextItemWidth(ImGui::GetContentRegionAvail().x * 0.6f);
        bool enter = ImGui::InputText("City", &city_, ImGuiInputTextFlags_EnterReturnsTrue);
        ImGui::SameLine();
        ImGui::BeginDisabled(is_loading_);
        if (ImGui::Button("Find") || enter) {
            StartLoading(true);
        }
        ImGui::EndDisabled();
        ImGui::TextDisabled("%s (%.2f, %.2f)", location_name_.c_str(), latitude_, longitude_);
        if (ShowsPreview()) {
            ImGui::TextDisabled("Preview data. Turn on \"Live data\" to see real weather.");
        }

        std::string error;
        {
            std::lock_guard<std::mutex> lock(pending_mutex_);
            error = last_error_;
        }
        if (!error.empty()) {
            ImGui::TextColored(ImVec4(1.0f, 0.4f, 0.4f, 1.0f), "%s", error.c_str());
        }

        const char* units[] = { "Celsius", "Fahrenheit" };
        int unit = fahrenheit_ ? 1 : 0;
        if (ImGui::Combo("Units", &unit, units, IM_ARRAYSIZE(units))) {
            fahrenheit_ = unit == 1;
            loaded_once_ = false;
        }
        ImGui::SliderInt("Forecast days", &forecast_days_, 0, 6);
        ImGui::DragInt("Update (minutes)", &update_interval_min_, 1, 5, 360);

        if (!ShowsPreview()) {
            ImGui::BeginDisabled(is_loading_);
            if (ImGui::Button("Reload now")) {
                StartLoading(false);
            }
            ImGui::EndDisabled();
        }
    }

    nlohmann::json WeatherWidget::ToJson() const {
        auto json = Widget::ToJson();
        json["city"] = city_;
        json["location_name"] = location_name_;
        json["latitude"] = latitude_;
        json["longitude"] = longitude_;
        json["fahrenheit"] = fahrenheit_;
        json["update_interval_min"] = update_interval_min_;
        json["forecast_days"] = forecast_days_;
        return json;
    }

    void WeatherWidget::FromJson(const nlohmann::json& json) {
        Widget::FromJson(json);
        city_ = json.value("city", city_);
        location_name_ = json.value("location_name", location_name_);
        latitude_ = json.value("latitude", latitude_);
        longitude_ = json.value("longitude", longitude_);
        fahrenheit_ = json.value("fahrenheit", fahrenheit_);
        update_interval_min_ = std::max(5, json.value("update_interval_min", update_interval_min_));
        forecast_days_ = std::clamp(json.value("forecast_days", forecast_days_), 0, 6);
        loaded_once_ = false;
    }
}
