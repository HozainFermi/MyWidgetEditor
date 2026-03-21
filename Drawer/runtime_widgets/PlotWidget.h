#pragma once
#include "Widget.h"
#include "../managers/RuntimeWidgetFactory.h"
#include <vector>
#include <string>

namespace rn {

    struct ChartData {
        std::string name;
        std::vector<float> x_data;
        std::vector<float> y_data;
    };
    struct ChartMapping {
        std::string title_column;
        std::string x_column;
        std::string y_column;

        nlohmann::json ToJson() const {
            return {
                {"chart_title", title_column},
                {"x_key", x_column},
                {"y_key", y_column}
            };
        }
        void FromJson(const nlohmann::json& json) {
            title_column = json.value("chart_title", "");
            x_column = json.value("x_key", "");
            y_column = json.value("y_key", "");
        }

    };

    enum class PlotType {
        LinePlot,
        FilledLinePlot,
        ScatterPlot,
        BubblePlot,
        PieChart,
        RealTimePlot,
        CandleStickChart,
        None
    };

    class PlotWidget : public Widget {
    private:
        // Данные для графика        
        PlotType plot_type_{ 0 };

        std::string plot_name_ = "Plot";
        std::string x_plot_label_ = "x values";
        std::string y_plot_label_ = "y values";

        std::vector<ChartMapping> chart_configs_;
        std::vector<ChartData> all_charts_;

    public:
        PlotWidget();
        PlotWidget(const std::string& name, const ImVec2& pos);

        bool UpdateInteraction(int widget_id) override;
        void Render(ImDrawList* draw_list) override;
        void RenderContent(ImVec2& screen_min, ImVec2& screen_max) override;

        void FromJson(const nlohmann::json& json) override;

        // Обработка входящих данных от таблицы
        void OnInput(const std::string& from_widget_id, const std::string& from_port, const std::vector<WidgetValue>& value) override;
    };
}

