#pragma once
#include "Widget.h"
#include <map>
#include <functional>
#include <memory>
#include <string>
#include <json.hpp>

namespace wg {

    class WidgetFactory {
    private:
        // Два типа creator'ов:
        // 1. CreatorFromJson - создаёт из JSON (знает о FromJson)
        // 2. CreatorWithName - создаёт с именем и позицией
        using CreatorFromJson = std::function<std::unique_ptr<Widget>(const nlohmann::json&)>;
        using CreatorWithName = std::function<std::unique_ptr<Widget>(const std::string&, const ImVec2&)>;

        static std::map<std::string, CreatorFromJson>& GetJsonCreators() {
            static std::map<std::string, CreatorFromJson> creators;
            return creators;
        }

        static std::map<std::string, CreatorWithName>& GetNameCreators() {
            static std::map<std::string, CreatorWithName> creators;
            return creators;
        }

    public:
        // Регистрация класса виджета
        template<typename T>
        static bool RegisterWidget(const std::string& type_name) {
            // Регистрируем создание из JSON
            GetJsonCreators()[type_name] = [](const nlohmann::json& json) -> std::unique_ptr<Widget> {
                auto widget = std::make_unique<T>();
                widget->FromJson(json);                
                return widget;
                };

            // Регистрируем создание с именем и позицией
            GetNameCreators()[type_name] = [](const std::string& name, const ImVec2& pos) -> std::unique_ptr<Widget> {
                return std::make_unique<T>(name, pos);
                };

            return true;
        }

        // Создание из JSON
        static std::unique_ptr<Widget> CreateFromJson(const nlohmann::json& json) {
            std::string type = json.value("type", "");

            auto& creators = GetJsonCreators();
            auto it = creators.find(type);
            if (it != creators.end()) {
                return it->second(json);  // Вызываем creator, который сам вызовет FromJson
            }

            return nullptr;
        }

        // Создание с именем и позицией
        static std::unique_ptr<Widget> CreateWithName(const std::string& type_name,
            const std::string& name,
            const ImVec2& pos) {
            auto& creators = GetNameCreators();
            auto it = creators.find(type_name);
            if (it != creators.end()) {
                return it->second(name, pos);
            }

            return nullptr;
        }

        // Получить список типов
        static std::vector<std::string> GetRegisteredTypes() {
            std::vector<std::string> types;
            for (const auto& pair : GetJsonCreators()) {
                types.push_back(pair.first);
            }
            return types;
        }

        static bool IsTypeRegistered(const std::string& type_name) {
            return GetJsonCreators().find(type_name) != GetJsonCreators().end();
        }
    };

 

#define REGISTER_WIDGET(ClassName) \
        namespace { \
            static bool ClassName##_registered = []() -> bool { \
                return wg::WidgetFactory::RegisterWidget<ClassName>(#ClassName); \
            }(); \
        }
}