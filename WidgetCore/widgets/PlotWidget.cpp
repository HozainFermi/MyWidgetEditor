#include "PlotWidget.h"
#include <cmath>
#include <unordered_map>
#include <imgui_internal.h>
#include <imgui_stdlib.h>
#include <implot/implot.h>

namespace core {

    namespace {
        const char* kPlotNames[] = { "LinePlot", "FilledLinePlot", "ScatterPlot", "BubblePlot", "PieChart", "RealTimePlot", "CandleStickChart" };
    }

    PlotWidget::PlotWidget()
        : Widget(WidgetType::PLOT, ImVec2(300, 200)) {
    }

    void PlotWidget::RenderContent(const ImVec2& screen_min, const ImVec2& screen_max)
    {
        ImVec2 size(screen_max.x - screen_min.x - 10.0f, screen_max.y - screen_min.y - 10.0f);
        ImVec2 pos(screen_min.x + 5.0f, screen_min.y + 5.0f);

        ImGui::SetCursorScreenPos(pos);
        if (ImGui::BeginChild(("##plot_" + GetId()).c_str(), size, false,
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
        {
            if (ShowsPreview()) {
                RenderPreviewPlot();
            }
            else {
                RenderDataPlot();
            }
        }
        ImGui::EndChild();
    }

    void PlotWidget::RenderPreviewPlot()
    {
        // Демо-кривые, чтобы в редакторе было видно, как выглядит график
        static float xs1[1001], ys1[1001];
        for (int i = 0; i < 1001; ++i) {
            xs1[i] = i * 0.001f;
            ys1[i] = 0.5f + 0.5f * sinf(50 * (xs1[i] + (float)ImGui::GetTime() / 10));
        }
        static double xs2[20], ys2[20];
        for (int i = 0; i < 20; ++i) {
            xs2[i] = i * 1 / 19.0f;
            ys2[i] = xs2[i] * xs2[i];
        }

        if (ImPlot::BeginPlot(plot_name_.c_str())) {
            ImPlot::SetupAxes(x_plot_label_.c_str(), y_plot_label_.c_str());
            ImPlot::PlotLine("f(x)", xs1, ys1, 1001);
            ImPlot::PlotLine("g(x)", xs2, ys2, 20, {
                ImPlotProp_Marker, ImPlotMarker_Circle,
                ImPlotProp_Flags, ImPlotLineFlags_Segments
            });
            ImPlot::EndPlot();
        }
    }

    void PlotWidget::RenderDataPlot()
    {
        // Остальные типы графиков пока не реализованы - рисуем как LinePlot
        if (ImPlot::BeginPlot(plot_name_.c_str())) {
            ImPlot::SetupAxes(x_plot_label_.c_str(), y_plot_label_.c_str(), ImPlotAxisFlags_AutoFit, ImPlotAxisFlags_AutoFit);
            for (const auto& chart : all_charts_) {
                int count = (int)std::min(chart.x_data.size(), chart.y_data.size());
                ImPlot::PlotLine(chart.name.c_str(), chart.x_data.data(), chart.y_data.data(), count);
            }
            ImPlot::EndPlot();
        }
    }

    void PlotWidget::RenderProperties()
    {
        ImGui::Text("PlotWidget");
        ImGui::Separator();
        ImGui::Text("Inputs:");
        ImGui::BulletText("data (array of json objects)");
        ImGui::BulletText("refresh (event)");
        ImGui::Separator();
        ImGui::Text("Properties:");

        if (ImGui::BeginCombo("Plot type", kPlotNames[(int)plot_type_])) {
            for (int n = 0; n < IM_ARRAYSIZE(kPlotNames); n++) {
                const bool is_selected = ((int)plot_type_ == n);
                if (ImGui::Selectable(kPlotNames[n], is_selected)) {
                    plot_type_ = (PlotType)n;
                }
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        const float field_width = ImGui::GetContentRegionAvail().x * 0.6f;
        ImGui::SetNextItemWidth(field_width);
        ImGui::InputText("Plot name", &plot_name_);
        ImGui::SetNextItemWidth(field_width);
        ImGui::InputText("X axis label", &x_plot_label_);
        ImGui::SetNextItemWidth(field_width);
        ImGui::InputText("Y axis label", &y_plot_label_);

        ImGui::Spacing();
        ImGui::Text("Charts:");
        ImGui::SameLine();
        ImGui::TextDisabled("(title / x key / y key)");

        for (size_t i = 0; i < chart_configs_.size(); i++) {
            ImGui::PushID((int)i);
            ImGui::Text("Chart %d:", (int)i + 1);

            // Три поля делят ширину панели, кнопка удаления справа
            const float spacing = ImGui::GetStyle().ItemSpacing.x;
            const float remove_width = ImGui::CalcTextSize("Remove").x + ImGui::GetStyle().FramePadding.x * 2.0f;
            const float column_width = ImMax(60.0f, (ImGui::GetContentRegionAvail().x - remove_width - spacing * 3.0f) / 3.0f);

            ImGui::SetNextItemWidth(column_width);
            ImGui::InputTextWithHint("##title_column", "title", &chart_configs_[i].title_column);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(column_width);
            ImGui::InputTextWithHint("##x_column", "x key", &chart_configs_[i].x_column);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(column_width);
            ImGui::InputTextWithHint("##y_column", "y key", &chart_configs_[i].y_column);

            ImGui::SameLine();
            bool remove = ImGui::Button("Remove");
            ImGui::PopID();

            if (remove) {
                chart_configs_.erase(chart_configs_.begin() + i);
                i--;
            }
        }

        if (ImGui::Button("Add chart")) {
            chart_configs_.push_back({ "New title", "new x", "new y" });
        }
        ImGui::Separator();
    }

    std::vector<PortDesc> PlotWidget::GetInputPorts() const
    {
        return {
            PortDesc{ "data", "Data", "json", true },
            PortDesc{ "refresh", "Refresh", "event", true }
        };
    }

    void PlotWidget::OnInput(const std::string& to_port, const std::string& from_widget_id, const std::vector<WidgetValue>& value)
    {
        if (to_port != "data") {
            return;
        }

        // Графики всегда соответствуют текущим настройкам (их могли поменять в редакторе)
        all_charts_.resize(chart_configs_.size());
        for (size_t i = 0; i < chart_configs_.size(); ++i) {
            all_charts_[i].name = chart_configs_[i].title_column;
            all_charts_[i].x_data.clear();
            all_charts_[i].y_data.clear();
        }

        // Имя поля -> векторы, куда складывать его значения
        std::unordered_map<std::string, std::vector<std::vector<float>*>> column_to_vectors;
        for (size_t i = 0; i < chart_configs_.size(); ++i) {
            const auto& cfg = chart_configs_[i];
            if (!cfg.x_column.empty()) column_to_vectors[cfg.x_column].push_back(&all_charts_[i].x_data);
            if (!cfg.y_column.empty()) column_to_vectors[cfg.y_column].push_back(&all_charts_[i].y_data);
        }

        for (const auto& row : value) {
            if (!row.is_object()) continue;

            for (auto it = row.begin(); it != row.end(); ++it) {
                auto map_it = column_to_vectors.find(it.key());
                if (map_it == column_to_vectors.end()) continue;

                float val = 0.0f;
                const auto& cell = it.value();
                if (cell.is_number()) {
                    val = cell.get<float>();
                }
                else if (cell.is_string()) {
                    try { val = std::stof(cell.get<std::string>()); }
                    catch (...) { continue; }
                }
                else {
                    continue;
                }

                for (std::vector<float>* target : map_it->second) {
                    target->push_back(val);
                }
            }
        }
    }

    nlohmann::json PlotWidget::ToJson() const
    {
        auto json = Widget::ToJson();
        json["plot_type"] = static_cast<int>(plot_type_);
        json["plot_name"] = plot_name_;
        json["x_plot_label"] = x_plot_label_;
        json["y_plot_label"] = y_plot_label_;

        auto chart_configs = nlohmann::json::array();
        for (const auto& cfg : chart_configs_) {
            chart_configs.push_back(cfg.ToJson());
        }
        json["chart_configs"] = chart_configs;
        return json;
    }

    void PlotWidget::FromJson(const nlohmann::json& json)
    {
        Widget::FromJson(json);
        if (json.contains("plot_type")) { plot_type_ = static_cast<PlotType>(json["plot_type"].get<int>()); }
        plot_name_ = json.value("plot_name", plot_name_);
        x_plot_label_ = json.value("x_plot_label", x_plot_label_);
        y_plot_label_ = json.value("y_plot_label", y_plot_label_);

        if (json.contains("chart_configs") && json["chart_configs"].is_array()) {
            chart_configs_.clear();
            for (const auto& cfg_json : json["chart_configs"]) {
                ChartMapping mapping;
                mapping.FromJson(cfg_json);
                chart_configs_.push_back(mapping);
            }
        }
        all_charts_.clear();
    }
}
