#pragma once
#include "Widget.h"
#include "../managers/RuntimeWidgetFactory.h"
#include <string>
#include <deque>
#include <vector>

namespace rn {

    struct ToastStyle {
        ImU32 bg_color = IM_COL32(30, 30, 40, 230);
        ImU32 border_color = IM_COL32(100, 100, 120, 255);
        ImU32 text_color = IM_COL32(255, 255, 255, 255);
        float border_thickness = 2.0f;
    };

    enum class ToastOp {
        AnyRow = 0,
        ColumnExists,
        AnyValueLessThan,
        AnyValueGreaterThan,
        AnyValueEquals,
        AnyValueContains
    };

    struct ToastRule {
        bool enabled = true;
        std::string name = "Rule";

        // Condition
        ToastOp op = ToastOp::AnyRow;
        std::string column;
        float number = 0.0f;
        std::string text;

        // Toast
        std::string message = "Notification";
        float duration_sec = 3.0f;
        ToastStyle style{};
    };

    struct ToastInstance {
        std::string message;
        double created_at = 0.0;
        float duration_sec = 3.0f;
        float fade_out_sec = 0.35f;
        ToastStyle style{};
    };

    class NotificationWidget : public Widget {
    private:
        std::deque<ToastInstance> toasts_;
        std::vector<ToastRule> rules_;

        int max_number_of_toasts_ = 8;
        float toast_width_ = 260.0f;
        float toast_padding_ = 8.0f;
        float toast_gap_ = 6.0f;
        bool newest_on_top_ = true;
        bool stack_from_bottom_ = true;
        float dismiss_slide_px_ = 80.0f;
        
    public:
        NotificationWidget();
        NotificationWidget(const std::string& name, const ImVec2& pos);

        void Render(ImDrawList* draw_list) override;
        void RenderContent(ImVec2& screen_min, ImVec2& screen_max) override;        

        std::vector<PortDesc> GetInputPorts() const override;
        std::vector<PortDesc> GetOutputPorts() const override;
        void OnInput(const std::string& from_widget_id, const std::string& from_port, const std::vector<WidgetValue>& value) override;
       
        void FromJson(const nlohmann::json& json) override;

    private:
        void PushToast(std::string message, const ToastStyle& style, float duration_sec);
        void PruneExpired();
        bool EvaluateRule(const ToastRule& rule, const std::vector<WidgetValue>& value) const;
        static float TryParseNumber(const nlohmann::json& v, bool& ok);
        static ImU32 MultiplyAlpha(ImU32 c, float a);

    };
}