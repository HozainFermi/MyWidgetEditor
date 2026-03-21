#pragma once
#include "Widget.h"
#include <vector>
#include <string>
#include "WidgetFactory.h"

namespace wg {

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
        PlotType plot_type_{0};

        std::string plot_name_ = "Plot";
        std::string x_plot_label_="x values";
        std::string y_plot_label_ = "y values";
        
        std::vector<ChartMapping> chart_configs_;
        std::vector<ChartData> all_charts_;


    public:
        PlotWidget();
        PlotWidget(const std::string& name, const ImVec2& pos);

        void Render(ImDrawList* draw_list, const ImVec2& canvas_p0) override;
        void RenderContent(ImVec2& screen_min, ImVec2& screen_max) override;
        void RenderProperties() override;

        nlohmann::json ToJson() const override;
        void FromJson(const nlohmann::json& json) override;

        // Порты: один вход "data" (массив чисел), один вход "event" для обновления
        std::vector<PortDesc> GetInputPorts() const override;
        std::vector<PortDesc> GetOutputPorts() const override;
        void OnInput(const std::string& port, const WidgetValue& value) override;
    };
}

