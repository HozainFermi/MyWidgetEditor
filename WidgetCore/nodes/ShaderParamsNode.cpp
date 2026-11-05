#include "ShaderParamsNode.h"
#include <cmath>
#include <imgui.h>

namespace core {

    ShaderParamsNode::ShaderParamsNode()
        : Widget(WidgetType::CUSTOM, ImVec2(0, 0)) {
    }

    std::vector<PortDesc> ShaderParamsNode::GetInputPorts() const {
        return { PortDesc{ "params", "Params", "json", true } };
    }

    void ShaderParamsNode::OnInput(const std::string& to_port, const std::string&, const std::vector<WidgetValue>& value) {
        if (to_port != "params" || value.empty() || !value[0].is_object()) {
            return;
        }
        for (auto it = value[0].begin(); it != value[0].end(); ++it) {
            const auto& v = it.value();
            float number = 0.0f;
            if (v.is_number()) {
                number = v.get<float>();
            }
            else if (v.is_string()) {
                try { size_t used = 0; number = std::stof(v.get<std::string>(), &used); if (used == 0) continue; }
                catch (...) { continue; }
            }
            else {
                continue;
            }
            target_[it.key()] = number;
            if (!current_.count(it.key())) {
                current_[it.key()] = number;  // первое значение - сразу, без перехода от нуля
            }
        }
    }

    void ShaderParamsNode::Update() {
        const double now = ImGui::GetTime();
        const float dt = last_time_ < 0.0 ? 0.0f : (float)(now - last_time_);
        last_time_ = now;

        // Экспоненциальное сглаживание: за smoothing секунд проходится ~95% пути до цели
        const float k = smoothing_sec_ <= 0.0f ? 1.0f : 1.0f - std::exp(-3.0f * dt / smoothing_sec_);
        animating_ = false;
        for (const auto& [name, target] : target_) {
            float& value = current_[name];
            value += (target - value) * k;
            if (std::abs(target - value) > 1e-3f * (std::max)(1.0f, std::abs(target))) {
                animating_ = true;
            }
            else {
                value = target;
            }
        }

        if (std::map<std::string, float>* uniforms = Context().shader_uniforms) {
            for (const auto& [name, value] : current_) {
                (*uniforms)[name] = value;
            }
        }
    }

    void ShaderParamsNode::RenderProperties() {
        ImGui::Text("==Shader params==");
        ImGui::TextWrapped("Every number in the first input row becomes uniform float u_<name> in the window background shader.");
        ImGui::Separator();
        ImGui::DragFloat("Smoothing (s)", &smoothing_sec_, 0.05f, 0.0f, 10.0f, "%.2f");
        if (current_.empty()) {
            ImGui::TextDisabled("No data yet (turn on \"Live data\" to see it in the editor)");
        }
        for (const auto& [name, value] : current_) {
            ImGui::BulletText("u_%s = %.3f  (target %.3f)", name.c_str(), value, target_[name]);
        }
    }

    nlohmann::json ShaderParamsNode::ToJson() const {
        auto json = Widget::ToJson();
        json["smoothing_sec"] = smoothing_sec_;
        return json;
    }

    void ShaderParamsNode::FromJson(const nlohmann::json& json) {
        Widget::FromJson(json);
        smoothing_sec_ = (std::max)(0.0f, json.value("smoothing_sec", smoothing_sec_));
    }
}
