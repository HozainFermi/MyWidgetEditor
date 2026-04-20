#pragma once
#include "Widget.h"
#include <string>
#include <vector>
#include "Scene.hpp"
#include "WidgetFactory.h"


namespace wg {

    enum class PathType {
        Model,
        VertexShader,
        FragmentShader
    };

   
    class RenderWidget : public Widget {
    private:
        Styles::Scene scene_;

    public:
        int selectedMeshIndex=-1;

        RenderWidget();
        RenderWidget(const std::string& name, const ImVec2& pos);
        bool UpdateInteraction(const ImVec2& canvas_p0, const ImVec2& canvas_size, int widget_id) override;
        void Render(ImDrawList* draw_list, const ImVec2& canvas_p0) override;
        void RenderContent(ImVec2& screen_min, ImVec2& screen_max) override;

        void RenderProperties();

        void RenderSelectedMeshProps(int index);
        
        void AddItem();
        void RemoveItem();
               
        nlohmann::json ToJson() const override;
        void FromJson(const nlohmann::json& json) override;

        // Порты: вход для данных сцены (на будущее)
        std::vector<PortDesc> GetInputPorts() const override {
            return { PortDesc{ "scene", "Scene", "render_scene", true } };
        }
        std::vector<PortDesc> GetOutputPorts() const override { return {}; }

    private:
        void GeneratePreviewData();
        void ExplorerButton(std::string& currentPath, std::vector<std::filesystem::path>& paths, std::filesystem::path& folderPath, PathType type);

        std::string PathTypeToString(PathType& type) {
            switch (type) {
                case PathType::Model: return "Model";
                case PathType::VertexShader: return "VertexShader";
                case PathType::FragmentShader: return "FragmentShader";
            }
        }
    };
}