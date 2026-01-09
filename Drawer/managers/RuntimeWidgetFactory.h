#pragma once
#include <map>
#include <functional>
#include <memory>
#include <string>
#include <json.hpp>
#include "../runtime_widgets/Widget.h"


namespace rn {

    class RuntimeWidgetFactory {
    private:
        // Два типа creator'ов:
        // 1. CreatorFromJson - создаёт из JSON (знает о FromJson)
        // 2. CreatorWithName - создаёт с именем и позицией
        using CreatorFromJson = std::function<std::unique_ptr<Widget>(const nlohmann::json&)>;       

        static std::map<std::string, CreatorFromJson>& GetJsonCreators() {
            static std::map<std::string, CreatorFromJson> creators;
            return creators;
        }     
        

    public:

        // Регистрация класса виджета
        template<typename T>
        static bool RegisterWidget(const std::string& type_name) {
            // Регистрируем создание из JSON
            GetJsonCreators()[type_name] = [](const nlohmann::json& json) -> std::unique_ptr<Widget> {
                return std::make_unique<T>();
                };
            
            return true;
        }

        // Создание из JSON
        static std::unique_ptr<Widget> CreateFromJson(const nlohmann::json& json) {
            std::string type = json.value("widget_class", "");

            auto& creators = GetJsonCreators();
            auto it = creators.find(type);
            if (it != creators.end()) {
                return it->second(json);  // Вызываем creator, который сам вызовет FromJson
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
                return rn::RuntimeWidgetFactory::RegisterWidget<ClassName>(#ClassName); \
            }(); \
        }
}