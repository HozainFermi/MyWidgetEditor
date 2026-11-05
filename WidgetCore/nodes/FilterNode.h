#pragma once
#include "core/Condition.h"
#include "core/Widget.h"

namespace core {

    // Узел "если": делит пришедшие строки по условиям.
    // Вход "in"; выходы "yes" (строки, подходящие под условия) и "no" (остальные).
    // Оба выхода отправляются всегда, даже пустые - получатели видят, что подходящих строк больше нет
    class FilterNode : public Widget {
    public:
        FilterNode();

        bool IsLogicNode() const override { return true; }
        void RenderProperties() override;

        std::vector<PortDesc> GetInputPorts() const override;
        std::vector<PortDesc> GetOutputPorts() const override;
        void OnInput(const std::string& to_port, const std::string& from_widget_id, const std::vector<WidgetValue>& value) override;
        void OnPortsConnected() override;

        nlohmann::json ToJson() const override;
        void FromJson(const nlohmann::json& json) override;

    private:
        void Process();

        ConditionGroup conditions_;
        std::vector<WidgetValue> last_input_;  // чтобы пересчитать при смене условий
        bool has_input_ = false;
    };
}
