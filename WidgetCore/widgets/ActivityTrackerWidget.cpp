#include "ActivityTrackerWidget.h"
#include <algorithm>
#include <cstdlib>
#include <ctime>
#include <filesystem>
#include <fstream>
#include <iostream>
#include <vector>
#include <imgui_internal.h>
#include <imgui_stdlib.h>

#ifdef _WIN32
#define NOMINMAX
#define WIN32_LEAN_AND_MEAN
#include <windows.h>
#endif

namespace core {

    namespace {
        const int kKeepDays = 30;

        std::string TodayKey() {
            std::time_t now = std::time(nullptr);
            std::tm local{};
#ifdef _WIN32
            localtime_s(&local, &now);
#else
            localtime_r(&now, &local);
#endif
            char buf[16];
            std::strftime(buf, sizeof(buf), "%Y-%m-%d", &local);
            return buf;
        }

        // "1ч 05м", "12м", "40с"
        std::string FormatDuration(double seconds) {
            int total = (int)seconds;
            int h = total / 3600, m = (total % 3600) / 60, s = total % 60;
            char buf[32];
            if (h > 0) snprintf(buf, sizeof(buf), "%dч %02dм", h, m);
            else if (m > 0) snprintf(buf, sizeof(buf), "%dм", m);
            else snprintf(buf, sizeof(buf), "%dс", s);
            return buf;
        }

        // "chrome.exe" -> "Chrome"
        std::string PrettyAppName(std::string exe) {
            if (exe.size() > 4 && _stricmp(exe.c_str() + exe.size() - 4, ".exe") == 0) {
                exe.resize(exe.size() - 4);
            }
            if (!exe.empty() && exe[0] >= 'a' && exe[0] <= 'z') {
                exe[0] = (char)(exe[0] - 'a' + 'A');
            }
            return exe;
        }

#ifdef _WIN32
        std::string WideToUtf8(const std::wstring& wide) {
            if (wide.empty()) return {};
            int size = WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), (int)wide.size(), nullptr, 0, nullptr, nullptr);
            std::string out(size, '\0');
            WideCharToMultiByte(CP_UTF8, 0, wide.c_str(), (int)wide.size(), out.data(), size, nullptr, nullptr);
            return out;
        }

        // exe активного окна; пусто, если это наше окно или узнать не удалось
        std::string ForegroundApp() {
            HWND hwnd = GetForegroundWindow();
            if (!hwnd) return {};
            DWORD pid = 0;
            GetWindowThreadProcessId(hwnd, &pid);
            if (pid == 0 || pid == GetCurrentProcessId()) return {};

            HANDLE process = OpenProcess(PROCESS_QUERY_LIMITED_INFORMATION, FALSE, pid);
            if (!process) return {};
            wchar_t path[MAX_PATH];
            DWORD size = MAX_PATH;
            std::string name;
            if (QueryFullProcessImageNameW(process, 0, path, &size)) {
                name = WideToUtf8(std::filesystem::path(path).filename().wstring());
            }
            CloseHandle(process);
            return name;
        }

        double IdleSeconds() {
            LASTINPUTINFO info{ sizeof(LASTINPUTINFO) };
            if (!GetLastInputInfo(&info)) return 0.0;
            return (GetTickCount() - info.dwTime) / 1000.0;
        }
#else
        std::string ForegroundApp() { return {}; }
        double IdleSeconds() { return 0.0; }
#endif

        const ImU32 kBarColors[] = {
            IM_COL32(86, 156, 255, 255), IM_COL32(255, 159, 67, 255), IM_COL32(46, 204, 113, 255),
            IM_COL32(231, 76, 120, 255), IM_COL32(155, 89, 182, 255), IM_COL32(241, 196, 15, 255),
            IM_COL32(26, 188, 156, 255), IM_COL32(149, 165, 166, 255) };
    }

    ActivityTrackerWidget::ActivityTrackerWidget()
        : Widget(WidgetType::CUSTOM, ImVec2(300, 220)) {
        preview_ = {
            { "chrome.exe", 2 * 3600 + 10 * 60 }, { "Code.exe", 3600 + 45 * 60 }, { "Telegram.exe", 40 * 60 },
            { "steam.exe", 25 * 60 }, { "explorer.exe", 10 * 60 }, { "Spotify.exe", 7 * 60 } };
    }

    ActivityTrackerWidget::~ActivityTrackerWidget() {
        // Не теряем время, накопленное после последнего автосохранения
        if (IsTracking() && loaded_) {
            SaveFile();
        }
    }

    bool ActivityTrackerWidget::IsTracking() const {
        return tracking_;
    }

    const std::map<std::string, double>& ActivityTrackerWidget::Shown() const {
        return ShowsPreview() ? preview_ : usage_;
    }

    std::string ActivityTrackerWidget::DataFilePath() const {
        if (!data_file_.empty()) {
            return data_file_;
        }
        const char* base = std::getenv("LOCALAPPDATA");
        std::filesystem::path dir = base ? std::filesystem::path(base) : std::filesystem::temp_directory_path();
        return (dir / "MyWidgetEditor" / "activity.json").string();
    }

    void ActivityTrackerWidget::OnContextChanged() {
        tracking_ = !Context().is_editor;
        loaded_ = false;
    }

    void ActivityTrackerWidget::OnPortsConnected() {
        if (!ShowsPreview() && loaded_) {
            EmitUsage();
        }
    }

    std::vector<PortDesc> ActivityTrackerWidget::GetOutputPorts() const {
        return { PortDesc{ "usage", "Usage", "json", false } };
    }

    void ActivityTrackerWidget::LoadFile() {
        today_ = TodayKey();
        usage_.clear();
        other_days_ = nlohmann::json::object();

        std::ifstream file(DataFilePath());
        if (file.is_open()) {
            try {
                nlohmann::json json;
                file >> json;
                other_days_ = json.value("days", nlohmann::json::object());
            }
            catch (const std::exception& e) {
                std::cerr << "ActivityTracker: can't read " << DataFilePath() << ": " << e.what() << std::endl;
            }
        }
        if (other_days_.contains(today_)) {
            usage_ = other_days_[today_].get<std::map<std::string, double>>();
            other_days_.erase(today_);
        }
        loaded_ = true;
    }

    void ActivityTrackerWidget::SaveFile() {
        nlohmann::json days = other_days_;
        days[today_] = usage_;

        // Храним только последние kKeepDays дней (ключи YYYY-MM-DD сортируются как даты)
        while ((int)days.size() > kKeepDays) {
            days.erase(days.begin());
        }

        std::filesystem::path path = DataFilePath();
        std::error_code ec;
        std::filesystem::create_directories(path.parent_path(), ec);
        // Пишем во временный файл и подменяем: при падении не останется половины json
        std::filesystem::path tmp = path;
        tmp += ".tmp";
        {
            std::ofstream file(tmp);
            if (!file.is_open()) {
                std::cerr << "ActivityTracker: can't write " << tmp << std::endl;
                return;
            }
            file << nlohmann::json{ {"days", days} }.dump(2);
        }
        std::filesystem::rename(tmp, path, ec);
    }

    void ActivityTrackerWidget::EmitUsage() {
        std::vector<std::pair<std::string, double>> sorted(usage_.begin(), usage_.end());
        std::sort(sorted.begin(), sorted.end(), [](const auto& a, const auto& b) { return a.second > b.second; });

        std::vector<WidgetValue> rows;
        int rank = 1;
        for (const auto& [app, seconds] : sorted) {
            rows.push_back({ {"app", PrettyAppName(app)}, {"seconds", seconds}, {"minutes", seconds / 60.0}, {"rank", rank++} });
        }
        Emit("usage", rows);
        last_emit_ = std::chrono::steady_clock::now();
    }

    void ActivityTrackerWidget::Update() {
        if (ShowsPreview()) {
            return;
        }
        const auto now = std::chrono::steady_clock::now();

        if (!loaded_) {
            LoadFile();
            last_tick_ = last_save_ = last_read_ = now;
            EmitUsage();
            return;
        }

        if (!IsTracking()) {
            // Редактор: только перечитываем файл, который пишет запущенный виджет
            if (now - last_read_ >= std::chrono::seconds(10)) {
                LoadFile();
                last_read_ = now;
                EmitUsage();
            }
            return;
        }

        if (now - last_tick_ < std::chrono::seconds(1)) {
            return;
        }
        // Если программа долго не рисовала кадры (сон, свёрнутое окно), не засчитываем этот разрыв
        double elapsed = std::min(std::chrono::duration<double>(now - last_tick_).count(), 5.0);
        last_tick_ = now;

        // Новый день: сохраняем вчерашний и начинаем с нуля
        if (TodayKey() != today_) {
            SaveFile();
            LoadFile();
        }

        is_idle_ = IdleSeconds() >= idle_threshold_sec_;
        current_app_ = ForegroundApp();
        if (!is_idle_ && !current_app_.empty()) {
            usage_[current_app_] += elapsed;
        }

        // Drawer закрывают через TerminateProcess (деструктор не вызывается) - сохраняем почаще
        if (now - last_save_ >= std::chrono::seconds(15)) {
            SaveFile();
            last_save_ = now;
        }
        if (now - last_emit_ >= std::chrono::seconds(60)) {
            EmitUsage();
        }
    }

    void ActivityTrackerWidget::Render(ImDrawList* dl, const ImVec2& min, const ImVec2& max) {
        const auto& usage = Shown();
        const float width = max.x - min.x;
        const float k = std::clamp((max.y - min.y) / 220.0f, 0.6f, 3.0f);
        const float pad = 12.0f * k;
        ImFont* font = ImGui::GetFont();

        dl->PushClipRect(min, max, true);
        dl->AddRectFilledMultiColor(min, max, IM_COL32(28, 30, 46, 235), IM_COL32(28, 30, 46, 235),
            IM_COL32(18, 20, 32, 235), IM_COL32(18, 20, 32, 235));

        std::vector<std::pair<std::string, double>> sorted(usage.begin(), usage.end());
        std::sort(sorted.begin(), sorted.end(), [](const auto& a, const auto& b) { return a.second > b.second; });

        double total = 0.0;
        for (const auto& item : sorted) total += item.second;

        // Заголовок: всего за сегодня
        float y = min.y + pad;
        dl->AddText(font, 13.0f * k, ImVec2(min.x + pad, y), IM_COL32(255, 255, 255, 160), "Сегодня за компьютером");
        y += 16.0f * k;
        std::string total_text = FormatDuration(total);
        dl->AddText(font, 30.0f * k, ImVec2(min.x + pad, y), IM_COL32(255, 255, 255, 255), total_text.c_str());

        if (IsTracking() && !ShowsPreview() && is_idle_) {
            const char* idle = "пауза";
            float w = font->CalcTextSizeA(12.0f * k, FLT_MAX, 0.0f, idle).x;
            dl->AddText(font, 12.0f * k, ImVec2(max.x - pad - w, min.y + pad), IM_COL32(255, 200, 90, 220), idle);
        }
        y += 38.0f * k;

        if (sorted.empty()) {
            dl->AddText(font, 13.0f * k, ImVec2(min.x + pad, y), IM_COL32(255, 255, 255, 140),
                IsTracking() ? "Пока ничего не набралось" : "Нет данных. Запусти виджет, чтобы считать время");
            dl->PopClipRect();
            return;
        }

        // Остальное после top_count_ складываем в "Другое"
        const int shown = std::min(top_count_, (int)sorted.size());
        double rest = 0.0;
        for (size_t i = shown; i < sorted.size(); ++i) rest += sorted[i].second;

        // Полоска долей сверху
        const float strip_h = 6.0f * k;
        float x = min.x + pad;
        const float strip_w = width - pad * 2;
        for (int i = 0; i < shown; ++i) {
            float w = (float)(sorted[i].second / total) * strip_w;
            dl->AddRectFilled(ImVec2(x, y), ImVec2(x + w, y + strip_h), kBarColors[i % IM_ARRAYSIZE(kBarColors)]);
            x += w;
        }
        if (rest > 0) {
            dl->AddRectFilled(ImVec2(x, y), ImVec2(min.x + pad + strip_w, y + strip_h), IM_COL32(90, 90, 100, 255));
        }
        y += strip_h + 8.0f * k;

        // Строки: цветная точка, имя, полоска относительно лидера, время
        const float row_h = 17.0f * k;
        const double top_value = sorted[0].second;
        auto draw_row = [&](const std::string& name, double seconds, ImU32 color) {
            if (y + row_h > max.y - pad * 0.5f) return;
            const float text_size = 12.0f * k;
            dl->AddCircleFilled(ImVec2(min.x + pad + 4.0f * k, y + row_h * 0.45f), 3.5f * k, color);
            dl->AddText(font, text_size, ImVec2(min.x + pad + 12.0f * k, y), IM_COL32(235, 235, 240, 255), name.c_str());

            std::string time_text = FormatDuration(seconds);
            float time_w = font->CalcTextSizeA(text_size, FLT_MAX, 0.0f, time_text.c_str()).x;
            dl->AddText(font, text_size, ImVec2(max.x - pad - time_w, y), IM_COL32(200, 200, 210, 255), time_text.c_str());

            float bar_left = min.x + pad + width * 0.38f;
            float bar_right = max.x - pad - time_w - 8.0f * k;
            if (bar_right > bar_left) {
                float bar_w = (float)(seconds / top_value) * (bar_right - bar_left);
                float by = y + row_h * 0.3f;
                dl->AddRectFilled(ImVec2(bar_left, by), ImVec2(bar_right, by + 5.0f * k), IM_COL32(255, 255, 255, 25), 3.0f * k);
                dl->AddRectFilled(ImVec2(bar_left, by), ImVec2(bar_left + bar_w, by + 5.0f * k), color, 3.0f * k);
            }
            y += row_h;
        };

        for (int i = 0; i < shown; ++i) {
            draw_row(PrettyAppName(sorted[i].first), sorted[i].second, kBarColors[i % IM_ARRAYSIZE(kBarColors)]);
        }
        if (rest > 0) {
            draw_row("Другое", rest, IM_COL32(120, 120, 130, 255));
        }

        dl->PopClipRect();
    }

    void ActivityTrackerWidget::RenderProperties() {
        ImGui::Text("==Activity Tracker==");
        ImGui::Separator();
        ImGui::SliderInt("Top apps", &top_count_, 1, 12);
        ImGui::DragInt("Idle after (sec)", &idle_threshold_sec_, 5, 30, 1800);
        ImGui::InputTextWithHint("Data file", "default: %LOCALAPPDATA%/MyWidgetEditor/activity.json", &data_file_);
        if (ImGui::IsItemDeactivatedAfterEdit()) {
            loaded_ = false;
        }
        ImGui::TextDisabled("%s", DataFilePath().c_str());
        ImGui::TextWrapped("Time is counted only by the running widget (Drawer). With \"Live data\" the editor shows the same file.");
    }

    nlohmann::json ActivityTrackerWidget::ToJson() const {
        auto json = Widget::ToJson();
        json["top_count"] = top_count_;
        json["idle_threshold_sec"] = idle_threshold_sec_;
        json["data_file"] = data_file_;
        return json;
    }

    void ActivityTrackerWidget::FromJson(const nlohmann::json& json) {
        Widget::FromJson(json);
        top_count_ = std::clamp(json.value("top_count", top_count_), 1, 12);
        idle_threshold_sec_ = std::max(30, json.value("idle_threshold_sec", idle_threshold_sec_));
        data_file_ = json.value("data_file", data_file_);
        loaded_ = false;
    }
}
