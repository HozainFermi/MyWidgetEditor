#pragma once
#include "Widget.h"
#include "../managers/RuntimeWidgetFactory.h"
#include <vector>
#include <string>

namespace rn {

    class PlotWidget : public Widget {
    private:
        std::vector<float> y_values_;
        std::string column_name_ = "Value";
        //---------------------------------\\
        std::vector<float> y_values_;
        std::vector < std::vector<float> > xy_values_;
        std::string plot_name_ = "Plot";
        std::vector < std::tuple<std::string, std::string, std::string> > charts_;


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

