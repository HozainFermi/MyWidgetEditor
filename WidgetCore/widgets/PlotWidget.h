#pragma once
#include <string>
#include <vector>
#include "core/Widget.h"

namespace core {

    struct ChartData {
        std::string name;
        std::vector<float> x_data;
        std::vector<float> y_data;
    };

    // Какие поля входных JSON-строк идут на оси графика
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
    public:
        PlotWidget();

        void RenderContent(const ImVec2& min, const ImVec2& max) override;
        void RenderProperties() override;

        nlohmann::json ToJson() const override;
        void FromJson(const nlohmann::json& json) override;

        // Входы: "data" (массив JSON-объектов) и "refresh" (событие)
        std::vector<PortDesc> GetInputPorts() const override;
        void OnInput(const std::string& to_port, const std::string& from_widget_id, const std::vector<WidgetValue>& value) override;

    private:
        void RenderPreviewPlot();
        void RenderDataPlot();

        PlotType plot_type_ = PlotType::LinePlot;
        std::string plot_name_ = "Plot";
        std::string x_plot_label_ = "x values";
        std::string y_plot_label_ = "y values";

        std::vector<ChartMapping> chart_configs_;
        std::vector<ChartData> all_charts_;
    };
}
