#include "PlotWidget.h"
#include <imgui_internal.h>
#include <iostream>
#include "../../external/implot/implot.h"

namespace rn {

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

    bool PlotWidget::UpdateInteraction(int widget_id)
    {
        return Widget::UpdateInteraction(widget_id);
    }

    void PlotWidget::Render(ImDrawList* draw_list)
    {
        Widget::Render(draw_list);
    }

    void PlotWidget::RenderContent(ImVec2& screen_min, ImVec2& screen_max)
    {
        ImVec2 size(screen_max.x - screen_min.x - 10.0f,
                    screen_max.y - screen_min.y - 10.0f);
        ImVec2 pos(screen_min.x + 5.0f, screen_min.y + 5.0f);

        ImGui::SetNextWindowPos(pos);
        ImGui::SetNextWindowSize(size);

        if (ImGui::BeginChild(("##plot_" + GetId()).c_str(), size, false,
            ImGuiWindowFlags_NoScrollbar | ImGuiWindowFlags_NoScrollWithMouse))
        {            
            switch (plot_type_) {
            case PlotType::LinePlot:
                if (ImPlot::BeginPlot(plot_name_.c_str())) {
                    ImPlot::SetupAxes(x_plot_label_.c_str(), y_plot_label_.c_str());
                    
                    for (int i = 0; i < all_charts_.size();++i) {
                        ImPlot::PlotLine(all_charts_[i].name.c_str(), all_charts_[i].x_data.data(), all_charts_[i].y_data.data(), all_charts_[i].x_data.size());
                    }                    
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
           
            ImGui::EndChild();
        }
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
                all_charts_.push_back({map.title_column, std::vector<float>(), std::vector<float>() });

            }
        }
    }

    void PlotWidget::OnInput(const std::string& from_widget_id,const std::string& from_port, const std::vector<WidgetValue>& value)
    {                
        if (from_port != "data" ) return;

        // Очистка старых данных
        for (auto& chart : all_charts_) {
            chart.x_data.clear();
            chart.y_data.clear();
        }

        // Карта соответствий: Column Name -> Список указателей на векторы, куда пушить данные
        std::unordered_map<std::string, std::vector<std::vector<float>*> > column_to_vectors;

        for (size_t i = 0; i < chart_configs_.size(); ++i) {
            // all_charts_ и chart_configs_ синхронизированы по индексу
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

    //void FilledLinePlots() {       
    //    static double xs1[101], ys1[101], ys2[101], ys3[101];
    //    srand(0);
    //    for (int i = 0; i < 101; ++i) {
    //        xs1[i] = (float)i;
    //        ys1[i] = RandomRange(400.0, 450.0);
    //        ys2[i] = RandomRange(275.0, 350.0);
    //        ys3[i] = RandomRange(150.0, 225.0);
    //    }
    //    static bool show_lines = true;
    //    static bool show_fills = true;
    //    static float fill_ref = 0;
    //    static int shade_mode = 0;
    //    static ImPlotShadedFlags flags = 0;
    //    ImGui::Checkbox("Lines", &show_lines); ImGui::SameLine();
    //    ImGui::Checkbox("Fills", &show_fills);
    //    if (show_fills) {
    //        ImGui::SameLine();
    //        if (ImGui::RadioButton("To -INF", shade_mode == 0))
    //            shade_mode = 0;
    //        ImGui::SameLine();
    //        if (ImGui::RadioButton("To +INF", shade_mode == 1))
    //            shade_mode = 1;
    //        ImGui::SameLine();
    //        if (ImGui::RadioButton("To Ref", shade_mode == 2))
    //            shade_mode = 2;
    //        if (shade_mode == 2) {
    //            ImGui::SameLine();
    //            ImGui::SetNextItemWidth(100);
    //            ImGui::DragFloat("##Ref", &fill_ref, 1, -100, 500);
    //        }
    //    }

    //    if (ImPlot::BeginPlot("Stock Prices")) {
    //        ImPlot::SetupAxes("Days", "Price");
    //        ImPlot::SetupAxesLimits(0, 100, 0, 500);
    //        if (show_fills) {
    //            ImPlotSpec spec;
    //            spec.Flags = flags;
    //            spec.FillAlpha = 0.25f;
    //            ImPlot::PlotShaded("Stock 1", xs1, ys1, 101, shade_mode == 0 ? -INFINITY : shade_mode == 1 ? INFINITY : fill_ref, spec);
    //            ImPlot::PlotShaded("Stock 2", xs1, ys2, 101, shade_mode == 0 ? -INFINITY : shade_mode == 1 ? INFINITY : fill_ref, spec);
    //            ImPlot::PlotShaded("Stock 3", xs1, ys3, 101, shade_mode == 0 ? -INFINITY : shade_mode == 1 ? INFINITY : fill_ref, spec);
    //        }
    //        if (show_lines) {
    //            ImPlot::PlotLine("Stock 1", xs1, ys1, 101);
    //            ImPlot::PlotLine("Stock 2", xs1, ys2, 101);
    //            ImPlot::PlotLine("Stock 3", xs1, ys3, 101);
    //        }
    //        ImPlot::EndPlot();
    //    }
    //}

    

    

}

