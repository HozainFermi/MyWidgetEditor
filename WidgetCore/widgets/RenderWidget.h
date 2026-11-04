#pragma once
#include <filesystem>
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

        nlohmann::json ToJson() const override;
        void FromJson(const nlohmann::json& json) override;

        // Вход для данных сцены (на будущее)
        std::vector<PortDesc> GetInputPorts() const override {
            return { PortDesc{ "scene", "Scene", "render_scene", true } };
        }

    private:
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
