#pragma once
#include <string>
#include <vector>
#include "core/Widget.h"

namespace core {

    // Одна колонка результата: output = source [op operand].
    // operand - число или имя другой колонки. Без операции колонка просто копируется/переименовывается
    struct MapColumn {
        std::string output;
        std::string source;
        int op = 0;            // 0 нет, 1 +, 2 -, 3 *, 4 /
        std::string operand;
    };

    // Узел "колонки": выбрать, переименовать и вычислить колонки, например
    //   price_rub = price * 90,  name = product.
    // Вход "in", выход "out"
    class MapNode : public Widget {
    public:
        MapNode();

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
        WidgetValue MapRow(const WidgetValue& row) const;

        std::vector<MapColumn> columns_;
        bool keep_other_columns_ = false;  // оставить и колонки, которых нет в списке

        std::vector<WidgetValue> last_input_;
        bool has_input_ = false;
    };
}
