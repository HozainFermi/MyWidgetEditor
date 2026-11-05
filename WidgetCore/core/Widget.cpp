#include "Widget.h"
#include <algorithm>
#include "EventBus.h"

namespace core {

    int Widget::next_id_ = 0;

    namespace {
        // Контекст по умолчанию, пока менеджер не выставил свой: рантайм, живые данные
        const WidgetContext kDefaultContext{};
    }

    const char* WidgetTypeToString(WidgetType type) {
        switch (type) {
        case WidgetType::TEXT: return "TEXT";
        case WidgetType::INPUT_TEXT: return "INPUT_TEXT";
        case WidgetType::BUTTON: return "BUTTON";
        case WidgetType::SLIDER: return "SLIDER";
        case WidgetType::CHECKBOX: return "CHECKBOX";
        case WidgetType::COMBOBOX: return "COMBOBOX";
        case WidgetType::COLOR_PICKER: return "COLOR_PICKER";
        case WidgetType::PLOT: return "PLOT";
        case WidgetType::TABLE: return "TABLE";
        case WidgetType::IMAGE: return "IMAGE";
        case WidgetType::CONTAINER: return "CONTAINER";
        case WidgetType::CUSTOM: return "CUSTOM";
        }
        return "NONE";
    }

    Widget::Widget(WidgetType type, const ImVec2& default_size)
        : id_("widget_" + std::to_string(next_id_++)), type_(type), size_(default_size), context_(&kDefaultContext) {
    }

    void Widget::ReserveId(const std::string& id) {
        const std::string prefix = "widget_";
        if (id.rfind(prefix, 0) != 0) {
            return;
        }
        try {
            next_id_ = std::max(next_id_, std::stoi(id.substr(prefix.size())) + 1);
        }
        catch (...) {
            // id не вида widget_<число> - конфликтовать не с чем
        }
    }

    void Widget::UpdateHoverFade(bool hovered, float dt) {
        const float target = (fade_on_hover_ && hovered) ? fade_min_alpha_ : 1.0f;
        const float step = fade_speed_ * dt;
        if (opacity_ > target) {
            opacity_ = std::max(target, opacity_ - step);
        }
        else {
            opacity_ = std::min(target, opacity_ + step);
        }
    }

    bool Widget::IsFading() const {
        return opacity_ < 1.0f;
    }

    void Widget::SetContext(const WidgetContext* context) {
        context_ = context ? context : &kDefaultContext;
        OnContextChanged();
    }

    void Widget::Emit(const std::string& port, const std::vector<WidgetValue>& value) {
        if (context_->bus) {
            context_->bus->Post(WidgetMessage{ id_, port, value });
        }
    }

    nlohmann::json Widget::ToJson() const {
        return {
            {"id", id_},
            {"name", name_},
            {"widget_class", widget_class_},
            {"type", static_cast<int>(type_)},
            {"position", {position_.x, position_.y}},
            {"size", {size_.x, size_.y}},
            {"bg_color", bg_color_},
            {"min_width", min_width_},
            {"min_height", min_height_},
            {"visibility", visible_},
            {"fade_on_hover", fade_on_hover_},
            {"fade_min_alpha", fade_min_alpha_},
            {"fade_speed", fade_speed_}
        };
    }

    void Widget::FromJson(const nlohmann::json& json) {
        if (json.contains("id")) {
            id_ = json["id"].get<std::string>();
            ReserveId(id_);
        }
        if (json.contains("name")) name_ = json["name"].get<std::string>();
        if (json.contains("widget_class")) widget_class_ = json["widget_class"].get<std::string>();
        if (json.contains("type")) type_ = static_cast<WidgetType>(json["type"].get<int>());
        if (json.contains("position") && json["position"].is_array()) {
            position_.x = json["position"][0];
            position_.y = json["position"][1];
        }
        if (json.contains("size") && json["size"].is_array()) {
            size_.x = json["size"][0];
            size_.y = json["size"][1];
        }
        if (json.contains("bg_color")) bg_color_ = json["bg_color"];
        if (json.contains("min_width")) min_width_ = json["min_width"];
        if (json.contains("min_height")) min_height_ = json["min_height"];
        visible_ = json.value("visibility", visible_);
        fade_on_hover_ = json.value("fade_on_hover", fade_on_hover_);
        fade_min_alpha_ = std::clamp(json.value("fade_min_alpha", fade_min_alpha_), 0.0f, 1.0f);
        fade_speed_ = std::max(0.05f, json.value("fade_speed", fade_speed_));
    }
}
