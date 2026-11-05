#pragma once
#include <filesystem>
#include <map>
#include <string>
#include <vector>
#include <Scene.hpp>
#include "core/Widget.h"

namespace core {

    enum class PathType {
        Model,
        VertexShader,
        FragmentShader
    };

    // 3D-сцена (модели через assimp) в своём framebuffer
    class RenderWidget : public Widget {
    public:
        RenderWidget();

        void RenderContent(const ImVec2& min, const ImVec2& max) override;
        void RenderProperties() override;
        float GetDesiredFps() const override { return 30.0f; }  // модель вращается

        nlohmann::json ToJson() const override;
        void FromJson(const nlohmann::json& json) override;

        // Вход "params": первая строка данных управляет сценой.
        //   rotation_speed (град/с), scale, camera_x/y/z, camera_zoom - параметры сцены;
        //   любое числовое поле <name> уходит в шейдеры моделей как uniform float u_<name>
        std::vector<PortDesc> GetInputPorts() const override {
            return { PortDesc{ "params", "Params", "json", true } };
        }
        void OnInput(const std::string& to_port, const std::string& from_widget_id, const std::vector<WidgetValue>& value) override;

    private:
        // Параметры сцены (сохраняются; вход "params" их меняет)
        float rotation_speed_ = 30.0f;  // градусов в секунду
        float scale_ = 1.0f;
        float angle_ = 0.0f;            // текущий угол, не сохраняется
        double last_time_ = -1.0;
        // Числа со входа -> uniform u_<имя> в шейдерах моделей
        std::map<std::string, float> shader_params_;

        void ExplorerButton(std::string& current_path, std::vector<std::filesystem::path>& paths, const std::filesystem::path& folder, PathType type);

        Styles::Scene scene_;

        // Состояние панели свойств - у каждого виджета своё
        int selected_model_ = 0;
        float x_offset_ = 0.0f;
        float y_offset_ = 0.0f;
        std::string model_path_;
        std::string vert_path_;
        std::string frag_path_;
        std::vector<std::filesystem::path> model_paths_;
        std::vector<std::filesystem::path> vert_paths_;
        std::vector<std::filesystem::path> frag_paths_;
    };
}
