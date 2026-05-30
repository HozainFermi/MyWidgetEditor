#include "NotificationWidget.h"
#include <algorithm>
#include <imgui.h>

namespace rn {

	REGISTER_WIDGET(NotificationWidget);

	NotificationWidget::NotificationWidget()
	 :Widget("Notification", WidgetType::CUSTOM, ImVec2(20, 20), ImVec2(400, 300))		
	{
		SetWidgetClass("NotificationWidget");
        rules_.push_back(ToastRule{});
	}

	NotificationWidget::NotificationWidget(const std::string& name, const ImVec2& pos)
	 :Widget(name, WidgetType::CUSTOM, pos, ImVec2(400, 300))
	{
		SetWidgetClass("NotificationWidget");
        rules_.push_back(ToastRule{});
	}

	
	void NotificationWidget::Render(ImDrawList* draw_list)
	{
		Widget::Render(draw_list);
	}

	void NotificationWidget::RenderContent(ImVec2& screen_min, ImVec2& screen_max)
	{
        PruneExpired();

        ImDrawList* dl = ImGui::GetWindowDrawList();

        const float max_w = toast_width_;
        const float x_right = screen_max.x - toast_padding_;
        float y = stack_from_bottom_
            ? (screen_max.y - toast_padding_)
            : (screen_min.y + toast_padding_);

        auto draw_one = [&](const ToastInstance& t) {
            const float inner_pad = 8.0f;
            const float text_w = max_w - inner_pad * 2.0f;
            ImVec2 text_size = ImGui::CalcTextSize(t.message.c_str(), nullptr, false, text_w);
            float h = text_size.y + inner_pad * 2.0f;

            const double now = ImGui::GetTime();
            const double age = now - t.created_at;
            const double fade_t0 = (double)t.duration_sec;
            const double fade_t1 = fade_t0 + (double)t.fade_out_sec;
            float alpha = 1.0f;
            if (age > fade_t0) {
                const double u = (t.fade_out_sec > 0.0f) ? ((age - fade_t0) / (double)t.fade_out_sec) : 1.0;
                alpha = 1.0f - (float)ImClamp(u, 0.0, 1.0);
            }
            const float slide = (1.0f - alpha) * dismiss_slide_px_;

            ImVec2 p0, p1;
            if (stack_from_bottom_) {
                p1 = ImVec2(x_right + slide, y);
                p0 = ImVec2(x_right - max_w + slide, y - h);
                y -= h + toast_gap_;
            } else {
                p0 = ImVec2(x_right - max_w + slide, y);
                p1 = ImVec2(x_right + slide, y + h);
                y += h + toast_gap_;
            }

            dl->AddRectFilled(p0, p1, MultiplyAlpha(t.style.bg_color, alpha), 6.0f);
            if (t.style.border_thickness > 0.0f) {
                dl->AddRect(p0, p1, MultiplyAlpha(t.style.border_color, alpha), 6.0f, 0, t.style.border_thickness);
            }
            dl->AddText(nullptr, 0.0f, ImVec2(p0.x + inner_pad, p0.y + inner_pad), MultiplyAlpha(t.style.text_color, alpha), t.message.c_str(), nullptr, text_w);
        };

        if (newest_on_top_) {
            for (auto it = toasts_.rbegin(); it != toasts_.rend(); ++it) {
                draw_one(*it);
            }
        } else {
            for (const auto& t : toasts_) {
                draw_one(t);
            }
        }
	}

	std::vector<PortDesc> NotificationWidget::GetInputPorts() const
	{
		return {
			PortDesc{ .name="data",  .label="Data",  .type="json",  .is_input=true },
			PortDesc{ .name="event", .label="Event", .type="event", .is_input=true }
		};
	}

	std::vector<PortDesc> NotificationWidget::GetOutputPorts() const
	{
		return std::vector<PortDesc>();
	}

	void NotificationWidget::OnInput(const std::string& from_widget_id, const std::string& from_port, const std::vector<WidgetValue>& value)
	{
        //(void)from_widget_id;
        std::cout << "NotificationWidget::OnInput" << std::endl;

        if (from_port == "event") {
            bool matched = false;
            for (const auto& r : rules_) {
                if (!r.enabled) continue;
                if (EvaluateRule(r, value)) {
                    PushToast(r.message, r.style, r.duration_sec);
                    matched = true;
                }
            }
            if (!matched) {
                ToastStyle s;
                PushToast("Event", s, 2.0f);
            }
            return;
        }

        if (from_port != "data") return;

        for (const auto& r : rules_) {
            if (!r.enabled) continue;
            if (EvaluateRule(r, value)) {
                PushToast(r.message, r.style, r.duration_sec);
            }
        }
	}
	
	void NotificationWidget::FromJson(const nlohmann::json& json)
	{
        Widget::FromJson(json);

        max_number_of_toasts_ = json.value("max_number_of_toasts", max_number_of_toasts_);
        toast_width_ = json.value("toast_width", toast_width_);
        toast_padding_ = json.value("toast_padding", toast_padding_);
        toast_gap_ = json.value("toast_gap", toast_gap_);
        newest_on_top_ = json.value("newest_on_top", newest_on_top_);

        rules_.clear();
        if (json.contains("rules") && json["rules"].is_array()) {
            for (const auto& rj : json["rules"]) {
                ToastRule r;
                r.enabled = rj.value("enabled", true);
                r.name = rj.value("name", "Rule");
                r.op = (ToastOp)rj.value("op", (int)ToastOp::AnyRow);
                r.column = rj.value("column", "");
                r.number = rj.value("number", 0.0f);
                r.text = rj.value("text", "");
                r.message = rj.value("message", "Notification");
                r.duration_sec = rj.value("duration_sec", 3.0f);

                if (rj.contains("style")) {
                    const auto& sj = rj["style"];
                    r.style.bg_color = sj.value("bg", r.style.bg_color);
                    r.style.border_color = sj.value("border", r.style.border_color);
                    r.style.text_color = sj.value("text", r.style.text_color);
                    r.style.border_thickness = sj.value("thickness", r.style.border_thickness);
                }

                rules_.push_back(std::move(r));
            }
        }
        if (rules_.empty()) rules_.push_back(ToastRule{});
	}

    void NotificationWidget::PushToast(std::string message, const ToastStyle& style, float duration_sec)
    {
        ToastInstance t;
        t.message = std::move(message);
        t.style = style;
        t.duration_sec = std::max(0.1f, duration_sec);
        t.created_at = ImGui::GetTime();

        toasts_.push_back(std::move(t));
        while ((int)toasts_.size() > max_number_of_toasts_) {
            toasts_.pop_front();
        }
    }

    void NotificationWidget::PruneExpired()
    {
        const double now = ImGui::GetTime();
        while (!toasts_.empty()) {
            const auto& t = toasts_.front();
            const double ttl = (double)t.duration_sec + (double)t.fade_out_sec;
            if (now - t.created_at > ttl) {
                toasts_.pop_front();
                continue;
            }
            break;
        }
    }

    ImU32 NotificationWidget::MultiplyAlpha(ImU32 c, float a)
    {
        ImVec4 v = ImGui::ColorConvertU32ToFloat4(c);
        v.w *= ImClamp(a, 0.0f, 1.0f);
        return ImGui::ColorConvertFloat4ToU32(v);
    }

    float NotificationWidget::TryParseNumber(const nlohmann::json& v, bool& ok)
    {
        ok = false;
        try {
            if (v.is_number()) {
                ok = true;
                return v.get<float>();
            }
            if (v.is_string()) {
                const auto s = v.get<std::string>();
                size_t idx = 0;
                float x = std::stof(s, &idx);
                if (idx > 0) {
                    ok = true;
                    return x;
                }
            }
        } catch (...) {
        }
        return 0.0f;
    }

    bool NotificationWidget::EvaluateRule(const ToastRule& rule, const std::vector<WidgetValue>& value) const
    {
        if (!rule.enabled) return false;

        if (rule.op == ToastOp::AnyRow) {
            return !value.empty();
        }

        if (rule.op == ToastOp::ColumnExists) {
            for (const auto& row : value) {
                if (!row.is_object()) continue;
                if (row.contains(rule.column)) return true;
            }
            return false;
        }

        for (const auto& row : value) {
            if (!row.is_object()) continue;
            if (!row.contains(rule.column)) continue;
            const auto& cell = row.at(rule.column);

            if (rule.op == ToastOp::AnyValueContains) {
                const auto s = cell.is_string() ? cell.get<std::string>() : cell.dump();
                if (!rule.text.empty() && s.find(rule.text) != std::string::npos) return true;
            }

            if (rule.op == ToastOp::AnyValueEquals) {
                bool ok_n = false;
                float n = TryParseNumber(cell, ok_n);
                if (ok_n) {
                    if (n == rule.number) return true;
                } else if (cell.is_string()) {
                    if (cell.get<std::string>() == rule.text) return true;
                } else {
                    if (cell.dump() == rule.text) return true;
                }
            }

            if (rule.op == ToastOp::AnyValueLessThan || rule.op == ToastOp::AnyValueGreaterThan) {
                bool ok_n = false;
                float n = TryParseNumber(cell, ok_n);
                if (!ok_n) continue;
                if (rule.op == ToastOp::AnyValueLessThan && n < rule.number) return true;
                if (rule.op == ToastOp::AnyValueGreaterThan && n > rule.number) return true;
            }
        }

        return false;
    }

	


}