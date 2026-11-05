#include "Condition.h"
#include <algorithm>
#include <imgui.h>
#include <imgui_stdlib.h>

namespace core {

    namespace {
        const char* kOpNames[] = { "==", "!=", ">", ">=", "<", "<=", "contains", "not contains", "exists", "missing" };

        bool ParseNumber(const std::string& text, double& out) {
            if (text.empty()) return false;
            try {
                size_t used = 0;
                out = std::stod(text, &used);
                return used == text.size();
            }
            catch (...) {
                return false;
            }
        }

        std::string CellText(const nlohmann::json& cell) {
            return cell.is_string() ? cell.get<std::string>() : cell.dump();
        }
    }

    bool Condition::Matches(const nlohmann::json& row) const {
        const bool has = row.is_object() && row.contains(column);
        if (op == ConditionOp::Exists) return has;
        if (op == ConditionOp::Missing) return !has;
        if (!has) return false;

        const nlohmann::json& cell = row.at(column);
        const std::string text = CellText(cell);

        if (op == ConditionOp::Contains) return text.find(value) != std::string::npos;
        if (op == ConditionOp::NotContains) return text.find(value) == std::string::npos;

        // Числа сравниваем как числа ("9" < "10"), остальное - как текст
        double a = 0.0, b = 0.0;
        const bool numbers = (cell.is_number() ? (a = cell.get<double>(), true) : ParseNumber(text, a)) && ParseNumber(value, b);
        int cmp = numbers ? (a < b ? -1 : (a > b ? 1 : 0)) : text.compare(value);
        cmp = cmp < 0 ? -1 : (cmp > 0 ? 1 : 0);

        switch (op) {
        case ConditionOp::Equals:    return cmp == 0;
        case ConditionOp::NotEquals: return cmp != 0;
        case ConditionOp::Greater:   return cmp > 0;
        case ConditionOp::GreaterEq: return cmp >= 0;
        case ConditionOp::Less:      return cmp < 0;
        case ConditionOp::LessEq:    return cmp <= 0;
        default:                     return false;
        }
    }

    nlohmann::json Condition::ToJson() const {
        return { {"column", column}, {"op", (int)op}, {"value", value} };
    }

    void Condition::FromJson(const nlohmann::json& json) {
        column = json.value("column", "");
        op = (ConditionOp)std::clamp(json.value("op", 0), 0, (int)ConditionOp::Missing);
        value = json.value("value", "");
    }

    bool ConditionGroup::Matches(const nlohmann::json& row) const {
        if (conditions.empty()) return true;
        for (const auto& c : conditions) {
            const bool ok = c.Matches(row);
            if (match_all && !ok) return false;
            if (!match_all && ok) return true;
        }
        return match_all;
    }

    nlohmann::json ConditionGroup::ToJson() const {
        nlohmann::json list = nlohmann::json::array();
        for (const auto& c : conditions) list.push_back(c.ToJson());
        return { {"match_all", match_all}, {"conditions", list} };
    }

    void ConditionGroup::FromJson(const nlohmann::json& json) {
        match_all = json.value("match_all", true);
        conditions.clear();
        for (const auto& c : json.value("conditions", nlohmann::json::array())) {
            Condition condition;
            condition.FromJson(c);
            conditions.push_back(condition);
        }
    }

    bool ConditionGroup::RenderEditor() {
        bool changed = false;
        int mode = match_all ? 0 : 1;
        ImGui::TextUnformatted("Pass rows where");
        ImGui::SameLine();
        ImGui::SetNextItemWidth(90);
        if (ImGui::Combo("##match", &mode, "ALL\0ANY\0")) {
            match_all = mode == 0;
            changed = true;
        }
        ImGui::SameLine();
        ImGui::TextUnformatted("conditions are true:");

        for (size_t i = 0; i < conditions.size(); ++i) {
            Condition& c = conditions[i];
            ImGui::PushID((int)i);
            const float w = (ImGui::GetContentRegionAvail().x - 30.0f) / 3.0f;

            ImGui::SetNextItemWidth(w);
            changed |= ImGui::InputTextWithHint("##column", "column", &c.column);
            ImGui::SameLine();
            int op = (int)c.op;
            ImGui::SetNextItemWidth(w);
            if (ImGui::Combo("##op", &op, kOpNames, IM_ARRAYSIZE(kOpNames))) {
                c.op = (ConditionOp)op;
                changed = true;
            }
            ImGui::SameLine();
            const bool needs_value = c.op != ConditionOp::Exists && c.op != ConditionOp::Missing;
            ImGui::BeginDisabled(!needs_value);
            ImGui::SetNextItemWidth(w);
            changed |= ImGui::InputTextWithHint("##value", "value", &c.value);
            ImGui::EndDisabled();
            ImGui::SameLine();
            const bool remove = ImGui::Button("x");
            ImGui::PopID();

            if (remove) {
                conditions.erase(conditions.begin() + i);
                --i;
                changed = true;
            }
        }
        if (ImGui::Button("Add condition")) {
            conditions.push_back(Condition{});
            changed = true;
        }
        if (conditions.empty()) {
            ImGui::TextDisabled("No conditions: everything passes");
        }
        return changed;
    }
}
