#include "FilterNode.h"
#include <imgui.h>

namespace core {

    FilterNode::FilterNode()
        : Widget(WidgetType::CUSTOM, ImVec2(0, 0)) {
        conditions_.conditions.push_back(Condition{ "price", ConditionOp::Greater, "100" });
    }

    std::vector<PortDesc> FilterNode::GetInputPorts() const {
        return { PortDesc{ "in", "In", "json", true } };
    }

    std::vector<PortDesc> FilterNode::GetOutputPorts() const {
        return {
            PortDesc{ "yes", "Yes", "json", false },
            PortDesc{ "no", "No", "json", false }
        };
    }

    void FilterNode::OnInput(const std::string& to_port, const std::string&, const std::vector<WidgetValue>& value) {
        if (to_port != "in") return;
        last_input_ = value;
        has_input_ = true;
        Process();
    }

    void FilterNode::OnPortsConnected() {
        if (has_input_) Process();
    }

    void FilterNode::Process() {
        std::vector<WidgetValue> yes, no;
        for (const auto& row : last_input_) {
            (conditions_.Matches(row) ? yes : no).push_back(row);
        }
        Emit("yes", yes);
        Emit("no", no);
    }

    void FilterNode::RenderProperties() {
        ImGui::Text("==Filter==");
        ImGui::TextDisabled("Rows that match go to \"yes\", the rest to \"no\".");
        ImGui::Separator();
        if (conditions_.RenderEditor() && has_input_) {
            Process();  // сразу видно результат на подключённых виджетах
        }
        if (has_input_) {
            size_t passed = 0;
            for (const auto& row : last_input_) passed += conditions_.Matches(row) ? 1 : 0;
            ImGui::Separator();
            ImGui::Text("Last input: %zu rows -> yes: %zu, no: %zu", last_input_.size(), passed, last_input_.size() - passed);
        }
    }

    nlohmann::json FilterNode::ToJson() const {
        auto json = Widget::ToJson();
        json["filter"] = conditions_.ToJson();
        return json;
    }

    void FilterNode::FromJson(const nlohmann::json& json) {
        Widget::FromJson(json);
        if (json.contains("filter")) {
            conditions_.FromJson(json["filter"]);
        }
    }
}
