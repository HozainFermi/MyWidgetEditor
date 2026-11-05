#pragma once
#include <deque>
#include <string>
#include <vector>
#include "core/Widget.h"

namespace core {

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

    // Правило: если пришедшие данные подходят под условие - показать уведомление
    struct ToastRule {
        bool enabled = true;
        std::string name = "Rule";

        ToastOp op = ToastOp::AnyRow;
        std::string column;
        float number = 0.0f;
        std::string text;

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

    // Всплывающие уведомления. Входы:
    //  "data"  - массив JSON-строк, каждое включённое правило проверяется по ним;
    //  "event" - то же, но если ни одно правило не сработало, показывается общее "Event".
    class NotificationWidget : public Widget {
    public:
        NotificationWidget();

        void Render(ImDrawList* draw_list, const ImVec2& min, const ImVec2& max) override;
        void RenderProperties() override;
        // Пока уведомления видны, они плавно исчезают и уезжают - нужна анимация
        float GetDesiredFps() const override { return toasts_.empty() ? 2.0f : 30.0f; }

        std::vector<PortDesc> GetInputPorts() const override;
        void OnInput(const std::string& to_port, const std::string& from_widget_id, const std::vector<WidgetValue>& value) override;

        nlohmann::json ToJson() const override;
        void FromJson(const nlohmann::json& json) override;

    private:
        void PushToast(std::string message, const ToastStyle& style, float duration_sec);
        void PruneExpired();
        bool EvaluateRule(const ToastRule& rule, const std::vector<WidgetValue>& value) const;

        std::deque<ToastInstance> toasts_;
        std::vector<ToastRule> rules_;

        int max_number_of_toasts_ = 8;
        float toast_width_ = 260.0f;
        float toast_padding_ = 8.0f;
        float toast_gap_ = 6.0f;
        bool newest_on_top_ = true;
        bool stack_from_bottom_ = true;
        float dismiss_slide_px_ = 80.0f;
    };
}
