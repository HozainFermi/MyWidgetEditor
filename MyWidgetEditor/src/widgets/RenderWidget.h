#pragma once
#include "Widget.h"
#include <string>
#include <vector>
#include "Scene.hpp"
#include "WidgetFactory.h"
#include <shapes/cube.hpp>
#include <shapes/plane.hpp>
#include <shaders/shaderSources.hpp>

namespace wg {

   
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

    
        /*struct RenderColumnConfig {
            std::string header;
            std::string data_field;
            float width = 100.0f;
            bool sortable = false;

            nlohmann::json ToJson() const {
                return {
                    {"header", header},
                    {"width", width},
                    {"data_field", data_field},
                    {"sortable", sortable}
                };
            }

            void FromJson(const nlohmann::json& json) {
                header = json.value("header", "");
                width = json.value("width", 100.0f);
                data_field = json.value("data_field", "");
                sortable = json.value("sortable", false);
            }
        };*/
    };
}