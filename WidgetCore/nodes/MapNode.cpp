#include "MapNode.h"
#include <algorithm>
#include <imgui.h>
#include <imgui_stdlib.h>

namespace core {

    namespace {
        const char* kOps[] = { "(copy)", "+", "-", "*", "/" };

        bool ToNumber(const nlohmann::json& value, double& out) {
            if (value.is_number()) {
                out = value.get<double>();
                return true;
            }
            if (value.is_string()) {
                try {
                    size_t used = 0;
                    const std::string s = value.get<std::string>();
                    out = std::stod(s, &used);
                    return used > 0;
                }
                catch (...) {}
            }
            return false;
        }
    }

    MapNode::MapNode()
        : Widget(WidgetType::CUSTOM, ImVec2(0, 0)) {
        columns_.push_back(MapColumn{ "id", "id", 0, "" });
        columns_.push_back(MapColumn{ "price_x2", "price", 3, "2" });
    }

    std::vector<PortDesc> MapNode::GetInputPorts() const {
        return { PortDesc{ "in", "In", "json", true } };
    }

    std::vector<PortDesc> MapNode::GetOutputPorts() const {
        return { PortDesc{ "out", "Out", "json", false } };
    }

    void MapNode::OnInput(const std::string& to_port, const std::string&, const std::vector<WidgetValue>& value) {
        if (to_port != "in") return;
        last_input_ = value;
        has_input_ = true;
        Process();
    }

    void MapNode::OnPortsConnected() {
        if (has_input_) Process();
    }

    WidgetValue MapNode::MapRow(const WidgetValue& row) const {
        WidgetValue out = keep_other_columns_ && row.is_object() ? row : WidgetValue::object();
        if (!row.is_object()) return out;

        for (const auto& col : columns_) {
            if (col.output.empty() || !row.contains(col.source)) continue;
            const WidgetValue& src = row[col.source];
            if (col.op == 0) {
                out[col.output] = src;
                continue;
            }
            // Операнд - число или имя колонки этой же строки
            double a = 0.0, b = 0.0;
            bool ok_b = false;
            try { size_t used = 0; b = std::stod(col.operand, &used); ok_b = used > 0; } catch (...) {}
            if (!ok_b && row.contains(col.operand)) ok_b = ToNumber(row[col.operand], b);
            if (!ToNumber(src, a) || !ok_b) continue;

            switch (col.op) {
            case 1: out[col.output] = a + b; break;
            case 2: out[col.output] = a - b; break;
            case 3: out[col.output] = a * b; break;
            case 4: if (b != 0.0) out[col.output] = a / b; break;
            }
        }
        return out;
    }

    void MapNode::Process() {
        std::vector<WidgetValue> result;
        result.reserve(last_input_.size());
        for (const auto& row : last_input_) {
            result.push_back(MapRow(row));
        }
        Emit("out", result);
    }

    void MapNode::RenderProperties() {
        ImGui::Text("==Map columns==");
        ImGui::TextDisabled("output = source [op number or column]");
        ImGui::Separator();

        bool changed = ImGui::Checkbox("Keep other columns", &keep_other_columns_);
        for (size_t i = 0; i < columns_.size(); ++i) {
            MapColumn& c = columns_[i];
            ImGui::PushID((int)i);
            const float w = (ImGui::GetContentRegionAvail().x - 60.0f) / 4.0f;
            ImGui::SetNextItemWidth(w);
            changed |= ImGui::InputTextWithHint("##out", "output", &c.output);
            ImGui::SameLine(); ImGui::TextUnformatted("=");
            ImGui::SameLine(); ImGui::SetNextItemWidth(w);
            changed |= ImGui::InputTextWithHint("##src", "source", &c.source);
            ImGui::SameLine(); ImGui::SetNextItemWidth(w);
            changed |= ImGui::Combo("##op", &c.op, kOps, IM_ARRAYSIZE(kOps));
            ImGui::SameLine();
            ImGui::BeginDisabled(c.op == 0);
            ImGui::SetNextItemWidth(w);
            changed |= ImGui::InputTextWithHint("##operand", "2 or column", &c.operand);
            ImGui::EndDisabled();
            ImGui::SameLine();
            const bool remove = ImGui::Button("x");
            ImGui::PopID();
            if (remove) {
                columns_.erase(columns_.begin() + i);
                --i;
                changed = true;
            }
        }
        if (ImGui::Button("Add column")) {
            columns_.push_back(MapColumn{});
            changed = true;
        }
        if (changed && has_input_) {
            Process();
        }
        if (has_input_ && !last_input_.empty()) {
            ImGui::Separator();
            ImGui::TextDisabled("First row after mapping:");
            ImGui::TextWrapped("%s", MapRow(last_input_[0]).dump().c_str());
        }
    }

    nlohmann::json MapNode::ToJson() const {
        auto json = Widget::ToJson();
        nlohmann::json cols = nlohmann::json::array();
        for (const auto& c : columns_) {
            cols.push_back({ {"output", c.output}, {"source", c.source}, {"op", c.op}, {"operand", c.operand} });
        }
        json["columns"] = cols;
        json["keep_other_columns"] = keep_other_columns_;
        return json;
    }

    void MapNode::FromJson(const nlohmann::json& json) {
        Widget::FromJson(json);
        keep_other_columns_ = json.value("keep_other_columns", keep_other_columns_);
        if (json.contains("columns") && json["columns"].is_array()) {
            columns_.clear();
            for (const auto& c : json["columns"]) {
                columns_.push_back(MapColumn{ c.value("output", ""), c.value("source", ""),
                    std::clamp(c.value("op", 0), 0, 4), c.value("operand", "") });
            }
        }
    }
}
