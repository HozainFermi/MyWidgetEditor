#pragma once
#include "Widget.h"
#include <vector>
#include <string>
#include "WidgetFactory.h"

namespace wg {

    class PlotWidget : public Widget {
    private:
        // Данные для графика
        std::vector<float> y_values_;
        std::vector<float> x_values_;
        std::string column_name_ = "Value";

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

