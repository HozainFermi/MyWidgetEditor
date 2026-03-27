#pragma once
#include "Widget.h"
#include <string>
#include <vector>
#include "../managers/RuntimeWidgetFactory.h"
#include <Scene.hpp>


namespace rn {

    class RenderWidget : public Widget {
    private:
        Styles::Scene scene_;

    public:
        int selectedMeshIndex = -1;

        RenderWidget();
        RenderWidget(const std::string& name, const ImVec2& pos);       
        void Render(ImDrawList* draw_list) override;
        void RenderContent(ImVec2& screen_min, ImVec2& screen_max) override;
       
        void RenderSelectedMeshProps(int index);

        void AddItem();
        void RemoveItem();

        void FromJson(const nlohmann::json& json) override;

        // Порты: вход для данных сцены (на будущее)
        std::vector<PortDesc> GetInputPorts() const override {
            return { PortDesc{ "scene", "Scene", "render_scene", true } };
        }
        std::vector<PortDesc> GetOutputPorts() const override { return {}; }

    private:
        void GeneratePreviewData();

    };
}