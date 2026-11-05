#pragma once
#include <map>
#include <string>
#include "core/Widget.h"

namespace core {

    // Данные -> шейдер фона окна. Каждое числовое поле первой строки на входе "params"
    // становится uniform float u_<имя> (например, temperature -> u_temperature).
    // Значения меняются плавно за smoothing секунд, поэтому фон "перетекает", а не прыгает.
    class ShaderParamsNode : public Widget {
    public:
        ShaderParamsNode();

        bool IsLogicNode() const override { return true; }
        void Update() override;
        void RenderProperties() override;
        float GetDesiredFps() const override { return animating_ ? 30.0f : 2.0f; }

        std::vector<PortDesc> GetInputPorts() const override;
        void OnInput(const std::string& to_port, const std::string& from_widget_id, const std::vector<WidgetValue>& value) override;

        nlohmann::json ToJson() const override;
        void FromJson(const nlohmann::json& json) override;

    private:
        float smoothing_sec_ = 1.5f;
        std::map<std::string, float> target_;
        std::map<std::string, float> current_;
        bool animating_ = false;
        double last_time_ = -1.0;
    };
}
