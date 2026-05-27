#pragma once
#include "Widget.h"
#include "WidgetFactory.h"
#include <string>
#include <queue>
#include <Connections.h>

namespace wg {

    struct Toast {
        std::string content;

        ImU32 bg_color_ = IM_COL32(40, 40, 80, 255);
        ImU32 border_color_ = IM_COL32(100, 100, 100, 255);      
        ImU32 text_color_ = IM_COL32(255, 255, 255, 255);
        float border_thickness_ = 2.0f;
    };

    class NotificationWidget : public Widget {
    private:
        std::queue<Toast> toasts_;
        std::unordered_map<Connection, Toast> toast_types_;
        int vanishing_time_ = 10; //seconds
        int max_number_of_toasts_ = 10;

        float toast_size_ = 10;
        float toast_gap_ = 5;
        
    public:
        NotificationWidget();
        NotificationWidget(const std::string& name, const ImVec2& pos);

      
        void Render(ImDrawList* draw_list, const ImVec2& canvas_p0) override;
        void RenderContent(ImVec2& screen_min, ImVec2& screen_max) override;
        void RenderProperties() override;

        std::vector<PortDesc> GetInputPorts() const override;
        std::vector<PortDesc> GetOutputPorts() const override;
        void OnInput(const std::string& from_widget_id, const std::string& from_port, const std::vector<WidgetValue>& value) override;

        nlohmann::json ToJson() const;
        void FromJson(const nlohmann::json& json);

    private:
        

    };
}