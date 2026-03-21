#include "PlotWidget.h"
#include <imgui_internal.h>
#include <implot/implot.h>



namespace wg {

    REGISTER_WIDGET(PlotWidget);

    PlotWidget::PlotWidget()
        : Widget("Plot", WidgetType::PLOT, ImVec2(20, 20), ImVec2(300, 200))
    {
        SetWidgetClass("PlotWidget");
    }

    PlotWidget::PlotWidget(const std::string& name, const ImVec2& pos)
        : Widget(name, WidgetType::PLOT, pos, ImVec2(300, 200))
    {
        SetWidgetClass("PlotWidget");
    }

    void PlotWidget::Render(ImDrawList* draw_list, const ImVec2& canvas_p0)
    {
        Widget::Render(draw_list, canvas_p0);
    }

    void PlotWidget::RenderContent(ImVec2& screen_min, ImVec2& screen_max)
    {
        ImVec2 size(screen_max.x - screen_min.x - 10.0f,
                    screen_max.y - screen_min.y - 10.0f);
        ImVec2 pos(screen_min.x + 5.0f, screen_min.y + 5.0f);

        ImGui::SetNextWindowPos(pos);
        ImGui::SetNextWindowSize(size);
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

        if (ImGui::BeginChild(("##plot_" + GetId()).c_str(), size, false,
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
        {
            //if (!all_charts_.empty()) {
                
                switch (plot_type_) {
                    case PlotType::LinePlot:
                        if (ImPlot::BeginPlot("Line Plots")) {
                            ImPlot::SetupAxes("x", "y");
                            ImPlot::PlotLine("f(x)", xs1, ys1, 1001);
                            ImPlot::PlotLine("g(x)", xs2, ys2, 20, {
                                ImPlotProp_Marker, ImPlotMarker_Circle,
                                ImPlotProp_Flags, ImPlotLineFlags_Segments
                                });
                            ImPlot::EndPlot();
                        }
                        break;
                    case PlotType::FilledLinePlot:
                        //ImPlot::Demo_FilledLinePlots();
                        break;
                    case PlotType::ScatterPlot:
                        //ImPlot::Demo_ScatterPlots();
                        break;
                    case PlotType::BubblePlot:
                        //ImPlot::Demo_BubblePlots();
                        break;
                    case PlotType::PieChart:
                        //ImPlot::Demo_PieCharts();
                        break;                   
                }
            
            //else {
            //    ImGui::Text("No data");
            //}

            ImGui::EndChild();
        }
    }

    void PlotWidget::RenderProperties()
    {
        static char plot_name_buf[128];
        static char x_label_buf[128];
        static char y_label_buf[128];
        
        const char* plot_names[] = { "LinePlot", "FilledLinePlot", "ScatterPlot", "BubblePlot","PieChart","RealTimePlot","CandleStickChart"};

        ImGui::Text("PlotWidget");
        ImGui::Separator();
        ImGui::Text("Inputs:");
        ImGui::BulletText("data (array of numbers)");
        ImGui::BulletText("refresh (event)");
        ImGui::Separator();
        ImGui::Text("Properties:");


        if (ImGui::BeginCombo("Plot type", plot_names[(int)plot_type_])) {
            for (int n = 0; n < 7; n++) {
                const bool is_selected = ((int)plot_type_ == n);
                if (ImGui::Selectable(plot_names[n], is_selected)) {
                    plot_type_ = (PlotType)n;
                }
                // Авто-скролл к выбранному элементу
                if (is_selected) {
                    ImGui::SetItemDefaultFocus();
                }
            }
            ImGui::EndCombo();
        }

        ImGui::SetNextItemWidth(60);
        if(ImGui::InputText("Plot name", plot_name_buf, (sizeof(plot_name_buf)) ) ) {
            plot_name_ = plot_name_buf;
        }
        ImGui::SetNextItemWidth(60);
        if (ImGui::InputText("X axis lable", x_label_buf, (sizeof(x_label_buf)))) {
            x_plot_label_ = x_label_buf;
        }
        ImGui::SameLine();        ;
        ImGui::SetNextItemWidth(60);
        if (ImGui::InputText("Y axis lable", y_label_buf, (sizeof(y_label_buf)))) {
            y_plot_label_ = y_label_buf;
        }

        // Настройки графиков
        ImGui::Text("Charts:");
        for (size_t i = 0; i < chart_configs_.size(); i++) {

            char titile_buf[128];
            char x_column_buf[128];
            char y_column_buf[128];
            
            strncpy_s(titile_buf, chart_configs_[i].title_column.c_str(), sizeof(titile_buf));
            strncpy_s(x_column_buf, chart_configs_[i].x_column.c_str(), sizeof(x_column_buf));
            strncpy_s(y_column_buf, chart_configs_[i].y_column.c_str(), sizeof(y_column_buf));
            
            ImGui::PushID((int)i);

            ImGui::Text("Chart %d:", (int)i + 1);
            ImGui::SetNextItemWidth(50);           
            if (ImGui::InputText("##title_column", titile_buf, sizeof(titile_buf))) {
                chart_configs_[i].title_column= titile_buf;
            }

            //ImGui::DragFloat("##width", &columns_[i].width, 1.0f, 20.0f, 500.0f);
            ImGui::SameLine();
            ImGui::SetNextItemWidth(50);
            if (ImGui::InputText("##x_column", x_column_buf, sizeof(x_column_buf))) {
                chart_configs_[i].x_column = x_column_buf;
            }
            ImGui::SameLine();
            ImGui::SetNextItemWidth(50);
            if (ImGui::InputText("##y_column", y_column_buf, sizeof(y_column_buf))) {
                chart_configs_[i].y_column = y_column_buf;
            }

            ImGui::SameLine();
            if (ImGui::Button("Remove")) {                
                chart_configs_.erase(chart_configs_.begin() + i);
                i--; // Уменьшаем индекс после удаления
            }

            ImGui::PopID();
        }

        if (ImGui::Button("Add chart")) {            
            chart_configs_.push_back({"New titile","new x","new y"});
        }

        ImGui::Separator();
        
    }

    std::vector<PortDesc> PlotWidget::GetInputPorts() const
    {
        return {
            PortDesc("data","Data","json",true),
            PortDesc{ "refresh", "Refresh", "event", true }
        };
    }

    std::vector<PortDesc> PlotWidget::GetOutputPorts() const
    {
        return {};
    }

    void PlotWidget::OnInput(const std::string& port, const WidgetValue& value) {
        if (port != "data" || !value.is_array()) return;

        // Очистка старых данных
        for (auto& chart : all_charts_) {
            chart.x_data.clear();
            chart.y_data.clear();
        }

        // Карта соответствий: Column Name -> Список указателей на векторы, куда пушить данные
        std::unordered_map<std::string, std::vector<std::vector<float>*>> column_to_vectors;

        for (size_t i = 0; i < chart_configs_.size(); ++i) {
            // Допустим, all_charts_ и chart_configs_ синхронизированы по индексу
            if (i >= all_charts_.size()) break;

            const auto& cfg = chart_configs_[i];
            auto& chart = all_charts_[i];

            if (!cfg.x_column.empty()) column_to_vectors[cfg.x_column].push_back(&chart.x_data);
            if (!cfg.y_column.empty()) column_to_vectors[cfg.y_column].push_back(&chart.y_data);
        }

        // Один проход по JSON
        for (const auto& row : value) {
            if (!row.is_object()) continue;

            // Итерируемся по полям текущей строки JSON
            for (auto it = row.begin(); it != row.end(); ++it) {
                const std::string& key = it.key();

                // Проверяем, интересует ли нас эта колонка (быстрый поиск в hashmap)
                auto map_it = column_to_vectors.find(key);
                if (map_it != column_to_vectors.end()) {

                    // Парсим значение один раз для всех графиков, использующих эту колонку
                    float val = 0.0f;
                    const auto& cell = it.value();

                    if (cell.is_number()) {
                        val = cell.get<float>();
                    }
                    else if (cell.is_string()) {
                        try { val = std::stof(cell.get<std::string>()); }
                        catch (...) { continue; }
                    }
                    else { continue; }

                    // Раскладываем значение во все нужные векторы
                    for (std::vector<float>* target_vec : map_it->second) {
                        target_vec->push_back(val);
                    }
                }
            }
        }
    }


    
    nlohmann::json PlotWidget::ToJson() const
    {
        auto json = Widget::ToJson();        
        json["plot_type"] = static_cast<int>(plot_type_);
        json["x_plot_label"] = x_plot_label_;
        json["y_plot_label"] = y_plot_label_;

        auto chart_configs = nlohmann::json::array();
        for (auto& cfg : chart_configs_) {
            chart_configs.push_back(cfg.ToJson());
        }
        json["chart_configs"] = chart_configs;
        return json;
    }

    void PlotWidget::FromJson(const nlohmann::json& json)
    {
        Widget::FromJson(json);
        if (json.contains("plot_type")) { plot_type_ = static_cast<PlotType>(json["plot_type"].get<int>()); }
        if (json.contains("x_plot_label")) { x_plot_label_ = json["x_plot_label"]; }
        if (json.contains("y_plot_label")) { y_plot_label_ = json["y_plot_label"]; }

        if (json.contains("chart_configs") && json["chart_configs"].is_array()) {
            chart_configs_.clear();
            for (const auto& cfg_json : json["chart_configs"]) {
                ChartMapping map;
                map.FromJson(cfg_json);
                chart_configs_.push_back(map);
            }
        }        
    }
}

