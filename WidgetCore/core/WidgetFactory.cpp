#include "WidgetFactory.h"

namespace core {

    std::map<std::string, WidgetFactory::Creator>& WidgetFactory::Creators() {
        static std::map<std::string, Creator> creators;
        return creators;
    }

    std::unique_ptr<Widget> WidgetFactory::Create(const std::string& class_name) {
        auto it = Creators().find(class_name);
        if (it == Creators().end()) {
            return nullptr;
        }
        std::unique_ptr<Widget> widget = it->second();
        widget->SetWidgetClass(class_name);
        return widget;
    }

    std::unique_ptr<Widget> WidgetFactory::CreateFromJson(const nlohmann::json& json) {
        std::unique_ptr<Widget> widget = Create(json.value("widget_class", ""));
        if (widget) {
            widget->FromJson(json);
        }
        return widget;
    }

    std::vector<std::string> WidgetFactory::GetRegisteredTypes() {
        std::vector<std::string> types;
        for (const auto& pair : Creators()) {
            types.push_back(pair.first);
        }
        return types;
    }

    bool WidgetFactory::IsTypeRegistered(const std::string& class_name) {
        return Creators().count(class_name) > 0;
    }
}
